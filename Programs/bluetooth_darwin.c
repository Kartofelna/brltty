/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2013 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#include "prologue.h"

#include <string.h>
#include <errno.h>

#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothSDPUUID.h>
#import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>
#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>

#include "log.h"
#include "io_misc.h"
#include "io_bluetooth.h"
#include "bluetooth_internal.h"
#include "system_darwin.h"

@interface ServiceQueryResult: AsynchronousResult
  {
  }

- (void) sdpQueryComplete
  : (IOBluetoothDevice *) device
  status: (IOReturn) status;
@end

@interface BluetoothConnectionDelegate: AsynchronousTask
  {
    BluetoothConnectionExtension *bluetoothConnectionExtension;
  }

- (void) setBluetoothConnectionExtension
  : (BluetoothConnectionExtension*) bcx;

- (BluetoothConnectionExtension*) getBluetoothConnectionExtension;
@end

@interface RfcommChannelDelegate: BluetoothConnectionDelegate
  {
    CFRunLoopRef runLoop;
  }

- (void) rfcommChannelData
  : (IOBluetoothRFCOMMChannel *) rfcommChannel
  data: (void *) dataPointer
  length: (size_t) dataLength;

- (void) run;

- (void) stop;

- (id) init;
@end

struct BluetoothConnectionExtensionStruct {
  BluetoothDeviceAddress bluetoothAddress;
  IOBluetoothDevice *bluetoothDevice;

  IOBluetoothRFCOMMChannel *rfcommChannel;
  RfcommChannelDelegate *rfcommDelegate;

  int inputPipe[2];
};

static void
bthSetError (IOReturn result, const char *action) {
  setDarwinSystemError(result);
  logSystemError(action);
}

static void
bthMakeAddress (BluetoothDeviceAddress *address, uint64_t bda) {
  unsigned int index = sizeof(address->data);

  while (index > 0) {
    address->data[--index] = bda & 0XFF;
    bda >>= 8;
  }
}

static int
bthPerformServiceQuery (BluetoothConnectionExtension *bcx) {
  int ok = 0;
  IOReturn result;
  ServiceQueryResult *target = [ServiceQueryResult new];

  if (target) {
    if ((result = [bcx->bluetoothDevice performSDPQuery:target]) == kIOReturnSuccess) {
      if ([target wait:10]) {
        if ((result = [target getStatus]) == kIOReturnSuccess) {
          ok = 1;
        } else {
          bthSetError(result, "service discovery response");
        }
      }
    } else {
      bthSetError(result, "service discovery request");
    }

    [target release];
  }

  return ok;
}

static int
bthGetServiceChannel (
  uint8_t *channel, BluetoothConnectionExtension *bcx,
  const void *uuidBytes, size_t uuidLength
) {
  IOReturn result;

  if (bthPerformServiceQuery(bcx)) {
    IOBluetoothSDPUUID *uuid = [IOBluetoothSDPUUID uuidWithBytes:uuidBytes length:uuidLength];

    if (uuid) {
      IOBluetoothSDPServiceRecord *record = [bcx->bluetoothDevice getServiceRecordForUUID:uuid];

      if (record) {
        if ((result = [record getRFCOMMChannelID:channel]) == kIOReturnSuccess) {
          return 1;
        } else {
          bthSetError(result, "RFCOMM channel lookup");
        }
      }
    }
  }

  return 0;
}

static int
bthGetSerialPortChannel (uint8_t *channel, BluetoothConnectionExtension *bcx) {
  return bthGetServiceChannel(channel, bcx, uuidBytes_serialPortProfile, uuidLength_serialPortProfile);
}

BluetoothConnectionExtension *
bthConnect (uint64_t bda, uint8_t channel, int timeout) {
  IOReturn result;
  BluetoothConnectionExtension *bcx;

  if ((bcx = malloc(sizeof(*bcx)))) {
    memset(bcx, 0, sizeof(*bcx));
    bcx->inputPipe[0] = bcx->inputPipe[1] = -1;
    bthMakeAddress(&bcx->bluetoothAddress, bda);

    if (pipe(bcx->inputPipe) != -1) {
      if (setBlockingIo(bcx->inputPipe[0], 0)) {
        if ((bcx->bluetoothDevice = [IOBluetoothDevice deviceWithAddress:&bcx->bluetoothAddress])) {
          [bcx->bluetoothDevice retain];

          if ((bcx->rfcommDelegate = [RfcommChannelDelegate new])) {
            [bcx->rfcommDelegate setBluetoothConnectionExtension:bcx];

            bthGetSerialPortChannel(&channel, bcx);
            logMessage(LOG_DEBUG, "using RFCOMM channel %u", channel);

            if ((result = [bcx->bluetoothDevice openRFCOMMChannelSync:&bcx->rfcommChannel withChannelID:channel delegate:nil]) == kIOReturnSuccess) {
              if ([bcx->rfcommDelegate start]) {
                return bcx;
              }

              [bcx->rfcommChannel closeChannel];
              [bcx->rfcommChannel release];
            } else {
              bthSetError(result, "RFCOMM channel open");
            }

            [bcx->rfcommDelegate stop];
            [bcx->rfcommDelegate release];
          }

          [bcx->bluetoothDevice closeConnection];
          [bcx->bluetoothDevice release];
        }
      }

      close(bcx->inputPipe[0]);
      close(bcx->inputPipe[1]);
    } else {
      logSystemError("pipe");
    }

    free(bcx);
  } else {
    logMallocError();
  }

  return NULL;
}

void
bthDisconnect (BluetoothConnectionExtension *bcx) {
  [bcx->rfcommChannel closeChannel];
  [bcx->rfcommChannel release];

  [bcx->rfcommDelegate stop];
  [bcx->rfcommDelegate release];

  [bcx->bluetoothDevice closeConnection];
  [bcx->bluetoothDevice release];

  close(bcx->inputPipe[0]);
  close(bcx->inputPipe[1]);

  free(bcx);
}

int
bthAwaitInput (BluetoothConnection *connection, int milliseconds) {
  BluetoothConnectionExtension *bcx = connection->extension;

  return awaitFileInput(bcx->inputPipe[0], milliseconds);
}

ssize_t
bthReadData (
  BluetoothConnection *connection, void *buffer, size_t size,
  int initialTimeout, int subsequentTimeout
) {
  BluetoothConnectionExtension *bcx = connection->extension;

  return readFile(bcx->inputPipe[0], buffer, size, initialTimeout, subsequentTimeout);
}

ssize_t
bthWriteData (BluetoothConnection *connection, const void *buffer, size_t size) {
  BluetoothConnectionExtension *bcx = connection->extension;
  IOReturn result = [bcx->rfcommChannel writeSync:(void *)buffer length:size];

  if (result == kIOReturnSuccess) return size;
  bthSetError(result, "RFCOMM channel write");
  return -1;
}

char *
bthObtainDeviceName (uint64_t bda, int timeout) {
  IOReturn result;
  BluetoothDeviceAddress address;

  bthMakeAddress(&address, bda);

  {
    IOBluetoothDevice *device = [IOBluetoothDevice deviceWithAddress:&address];

    if (device != nil) {
      if ((result = [device remoteNameRequest:nil]) == kIOReturnSuccess) {
        NSString *nsName = device.name;

        if (nsName != nil) {
          const char *utf8Name = [nsName UTF8String];

          if (utf8Name != NULL) {
            char *name = strdup(utf8Name);

            if (name != NULL) {
              return name;
            }
          }
        }
      } else {
        bthSetError(result, "device name query");
      }

      [device closeConnection];
    }
  }

  return NULL;
}

@implementation ServiceQueryResult
- (void) sdpQueryComplete
  : (IOBluetoothDevice *) device
  status: (IOReturn) status
  {
    [self setStatus:status];
  }
@end

@implementation BluetoothConnectionDelegate
- (void) setBluetoothConnectionExtension
  : (BluetoothConnectionExtension*) bcx
  {
    bluetoothConnectionExtension = bcx;
  }

- (BluetoothConnectionExtension *) getBluetoothConnectionExtension
  {
    return bluetoothConnectionExtension;
  }
@end

@implementation RfcommChannelDelegate
- (void) rfcommChannelData
  : (IOBluetoothRFCOMMChannel *) rfcommChannel
  data: (void *) dataPointer
  length: (size_t) dataLength
  {
    writeFile(bluetoothConnectionExtension->inputPipe[1], dataPointer, dataLength);
  }

- (void) run
  {
    logMessage(LOG_DEBUG, "RFCOMM channel delegate started");

    {
      BluetoothConnectionExtension *bcx = bluetoothConnectionExtension;
      IOReturn result;

      if ((result = [bcx->rfcommChannel setDelegate:self]) == kIOReturnSuccess) {
        CFRunLoopRun();
      } else {
        bthSetError(result, "RFCOMM channel delegate set");
      }
    }

    logMessage(LOG_DEBUG, "RFCOMM channel delegate finished");
  }

- (void) stop
  {
    CFRunLoopStop(runLoop);
  }

- (id) init
  {
    if ((self = [super init])) {
      runLoop = CFRunLoopGetCurrent();
    }

    return self;
  }
@end

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
#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>

#include "log.h"
#include "io_misc.h"
#include "io_bluetooth.h"
#include "bluetooth_internal.h"
#include "system_darwin.h"

@interface AsynchronousOperation: NSObject {
  int isFinished;
  IOReturn finalStatus;
}

- (int)wait;
- (void)setStatus:(IOReturn)status;
- (IOReturn)getStatus;
@end

@interface BluetoothConnectionDelegate: AsynchronousOperation {
  BluetoothConnectionExtension *bluetoothConnectionExtension;
}

- (void)setBluetoothConnectionExtension:(BluetoothConnectionExtension*)bcx;
- (BluetoothConnectionExtension*)getBluetoothConnectionExtension;
@end

@interface SdpQueryDelegate: BluetoothConnectionDelegate {
}

- (void)sdpQueryComplete:(IOBluetoothDevice *)device status:(IOReturn)status;
@end

@interface RfcommChannelDelegate: BluetoothConnectionDelegate {
}

- (void)rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength;
@end

struct BluetoothConnectionExtensionStruct {
  BluetoothDeviceAddress address;
  IOBluetoothDevice *device;
  IOBluetoothRFCOMMChannel *channel;
  RfcommChannelDelegate *delegate;
  int inputPipe[2];
};

@implementation AsynchronousOperation
- (int)wait {
    isFinished = 0;

    while (1) {
      executeRunLoop(60);
      if (isFinished) return 1;
    }
  }

- (void)setStatus:(IOReturn)status {
    [self setStatus:status];
  }

- (IOReturn)getStatus {
    return finalStatus;
  }
@end

@implementation BluetoothConnectionDelegate
- (void)setBluetoothConnectionExtension:(BluetoothConnectionExtension*)bcx {
    bluetoothConnectionExtension = bcx;
  }

- (BluetoothConnectionExtension*)getBluetoothConnectionExtension {
    return bluetoothConnectionExtension;
  }
@end

@implementation SdpQueryDelegate
- (void)sdpQueryComplete:(IOBluetoothDevice *)device status:(IOReturn)status {
    isFinished = 1;
    finalStatus = status;
  }
@end

@implementation RfcommChannelDelegate
- (void)rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength {
    writeFile(bluetoothConnectionExtension->inputPipe[1], dataPointer, dataLength);
  }
@end

static void
bthMakeAddress (BluetoothDeviceAddress *address, uint64_t bda) {
  unsigned int index = sizeof(address->data);

  while (index > 0) {
    address->data[--index] = bda & 0XFF;
    bda >>= 8;
  }
}

static int
bthGetSerialPortChannel (uint8_t *channel, BluetoothConnectionExtension *bcx) {
  int found = 0;
  IOReturn result;
  SdpQueryDelegate *delegate = [SdpQueryDelegate new];

  if (delegate) {
    [delegate setBluetoothConnectionExtension:bcx];

    if ((result = [bcx->device performSDPQuery:delegate]) == kIOReturnSuccess) {
      if ([delegate wait]) {
        if ([delegate getStatus] == kIOReturnSuccess) {
          logMessage(LOG_NOTICE, "sdp done");
        }
      }
    } else {
      setDarwinSystemError(result);
    }

    [delegate release];
  }

  return found;
}

BluetoothConnectionExtension *
bthConnect (uint64_t bda, uint8_t channel, int timeout) {
  IOReturn result;
  BluetoothConnectionExtension *bcx;

  if ((bcx = malloc(sizeof(*bcx)))) {
    memset(bcx, 0, sizeof(*bcx));
    bcx->inputPipe[0] = bcx->inputPipe[1] = -1;
    bthMakeAddress(&bcx->address, bda);

    if (pipe(bcx->inputPipe) != -1) {
      if (setBlockingIo(bcx->inputPipe[0], 0)) {
        if ((bcx->device = [IOBluetoothDevice deviceWithAddress:&bcx->address])) {
          [bcx->device retain];

          if ((bcx->delegate = [RfcommChannelDelegate new])) {
            [bcx->delegate setBluetoothConnectionExtension:bcx];
            bthGetSerialPortChannel(&channel, bcx);
exit(0);

            if ((result = [bcx->device openRFCOMMChannelSync:&bcx->channel withChannelID:channel delegate:bcx->delegate]) == kIOReturnSuccess) {
              [bcx->channel closeChannel];
              [bcx->channel release];
            }

            [bcx->delegate release];
          }

          [bcx->device closeConnection];
          [bcx->device release];
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
  [bcx->channel closeChannel];
  [bcx->channel release];

  [bcx->delegate release];

  [bcx->device closeConnection];
  [bcx->device release];

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
  IOReturn result = [bcx->channel writeSync:(void*)buffer length:size];

  if (result == kIOReturnSuccess) return size;
  return -1;
}

char *
bthObtainDeviceName (uint64_t bda) {
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
      }

      [device closeConnection];
    }
  }

  return NULL;
}

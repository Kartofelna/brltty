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

#include "log.h"
#include "async.h"
#include "parse.h"
#include "device.h"
#include "queue.h"
#include "io_bluetooth.h"
#include "bluetooth_internal.h"

const uint8_t uuidBytes_serialPortProfile[] = {
  0X00, 0X00, 0X11, 0X01,
  0X00, 0X00,
  0X10, 0X00,
  0X80, 0X00,
  0X00, 0X80, 0X5F, 0X9B, 0X34, 0XFB
};
const uint8_t uuidLength_serialPortProfile = ARRAY_COUNT(uuidBytes_serialPortProfile);

int
bthDiscoverSerialPortChannel (uint8_t *channel, BluetoothConnectionExtension *bcx) {
  return bthDiscoverChannel(channel, bcx, uuidBytes_serialPortProfile, uuidLength_serialPortProfile);
}

void
bthLogChannel (uint8_t channel) {
  logMessage(LOG_DEBUG, "using RFCOMM channel %u", channel);
}

typedef struct {
  uint64_t bda;
  int connectError;
  char *deviceName;
} BluetoothDeviceEntry;
static Queue *bluetoothDeviceQueue = NULL;

static void
bthDeallocateDeviceEntry (void *item, void *data) {
  BluetoothDeviceEntry *entry = item;

  if (entry->deviceName) free(entry->deviceName);
  free(entry);
}

static int
bthInitializeDeviceQueue (void) {
  if (!bluetoothDeviceQueue) {
    if (!(bluetoothDeviceQueue = newQueue(bthDeallocateDeviceEntry, NULL))) {
      return 0;
    }
  }

  return 1;
}

static int
bthTestDeviceEntry (const void *item, const void *data) {
  const BluetoothDeviceEntry *entry = item;
  const uint64_t *bda = data;

  return entry->bda == *bda;
}

static BluetoothDeviceEntry *
bthGetDeviceEntry (uint64_t bda, int add) {
  if (bthInitializeDeviceQueue()) {
    BluetoothDeviceEntry *entry = findItem(bluetoothDeviceQueue, bthTestDeviceEntry, &bda);
    if (entry) return entry;

    if (add) {
      if ((entry = malloc(sizeof(*entry)))) {
        entry->bda = bda;
        entry->connectError = 0;
        entry->deviceName = NULL;

        if (enqueueItem(bluetoothDeviceQueue, entry)) return entry;
        free(entry);
      } else {
        logMallocError();
      }
    }
  }

  return NULL;
}

void
bthClearCache (void) {
  if (bthInitializeDeviceQueue()) deleteElements(bluetoothDeviceQueue);
}

static int
bthRememberConnectError (uint64_t bda, int value) {
  BluetoothDeviceEntry *entry = bthGetDeviceEntry(bda, 1);
  if (!entry) return 0;

  entry->connectError = value;
  return 1;
}

static int
bthRecallConnectError (uint64_t bda, int *value) {
  BluetoothDeviceEntry *entry = bthGetDeviceEntry(bda, 0);
  if (!entry) return 0;
  if (!entry->connectError) return 0;

  *value = entry->connectError;
  return 1;
}

static int
bthParseAddress (uint64_t *bda, const char *address) {
  const char *character = address;
  const unsigned int width = 8;
  const unsigned long mask = (1UL << width) - 1UL;
  int counter = BDA_SIZE;
  *bda = 0;

  do {
    *bda <<= width;

    if (*character) {
      char *end;
      long int value = strtol(character, &end, 0X10);

      if (end == character) goto error;
      if (value < 0) goto error;
      if (value > mask) goto error;
      *bda |= value;

      if (!*(character = end)) continue;
      if (*character != ':') goto error;
      if (!*++character) goto error;
    }
  } while (--counter);

  if (!*character) return 1;

error:
  logMessage(LOG_ERR, "invalid Bluetooth device address: %s", address);
  errno = EINVAL;
  return 0;
}

BluetoothConnection *
bthOpenConnection (const char *identifier, uint8_t channel, int discover) {
  static const char *const parameterNames[] = {
    "address",
    NULL
  };

  enum {
    PARM_ADDRESS
  };

  char **parameterValues = getDeviceParameters(parameterNames, identifier);

  if (parameterValues) {
    BluetoothConnection *connection;

    if ((connection = malloc(sizeof(*connection)))) {
      memset(connection, 0, sizeof(*connection));
      connection->channel = channel;

      if (bthParseAddress(&connection->address, parameterValues[PARM_ADDRESS])) {
        int alreadyTried = 0;

        {
          int value;

          if (bthRecallConnectError(connection->address, &value)) {
            errno = value;
            alreadyTried = 1;
          }
        }

        if (!alreadyTried) {
          TimePeriod period;
          startTimePeriod(&period, 2000);

          while (1) {
            if ((connection->extension = bthConnect(connection->address, connection->channel, discover, 15000))) {
              deallocateStrings(parameterValues);
              return connection;
            }

            if (afterTimePeriod(&period, NULL)) break;
            if (errno != EBUSY) break;
            asyncWait(100);
          }

          bthRememberConnectError(connection->address, errno);
        }
      }

      free(connection);
    } else {
      logMallocError();
    }

    deallocateStrings(parameterValues);
  }

  return NULL;
}

void
bthCloseConnection (BluetoothConnection *connection) {
  bthDisconnect(connection->extension);
  free(connection);
}

static char *
bthGetDeviceName (uint64_t bda, int timeout) {
  BluetoothDeviceEntry *entry = bthGetDeviceEntry(bda, 1);

  if (entry) {
    if (!entry->deviceName) {
      entry->deviceName = bthObtainDeviceName(bda, timeout);
    }

    return entry->deviceName;
  }

  return NULL;
}

char *
bthGetNameOfDevice (BluetoothConnection *connection, int timeout) {
  return bthGetDeviceName(connection->address, timeout);
}

char *
bthGetNameAtAddress (const char *address, int timeout) {
  uint64_t bda;

  if (bthParseAddress(&bda, address)) {
    return bthGetDeviceName(bda, timeout);
  }

  return NULL;
}

const char *const *
bthGetDriverCodes (const char *address, int timeout) {
  const char *name = bthGetNameAtAddress(address, timeout);

  if (name) {
    const BluetoothNameEntry *entry = bluetoothNameTable;

    while (entry->namePrefix) {
      if (strncmp(name, entry->namePrefix, strlen(entry->namePrefix)) == 0) {
        return entry->driverCodes;
      }

      entry += 1;
    }
  }

  return NULL;
}

int
isBluetoothDevice (const char **identifier) {
  if (isQualifiedDevice(identifier, "bluetooth")) return 1;
  if (isQualifiedDevice(identifier, "bt")) return 1;
  if (isQualifiedDevice(identifier, "bluez")) return 1;
  return 0;
}

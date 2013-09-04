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

#ifndef BRLTTY_INCLUDED_IO_BLUETOOTH
#define BRLTTY_INCLUDED_IO_BLUETOOTH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct BluetoothConnectionStruct BluetoothConnection;

extern void bthClearCache (void);

extern char *bthGetNameOfDevice (BluetoothConnection *connection, int timeout);
extern char *bthGetNameAtAddress (const char *address, int timeout);
extern const char *const *bthGetDriverCodes (const char *address, int timeout);

typedef struct {
  const char *identifier;
  int timeout;
  uint8_t channel;
  unsigned discover:1;
} BluetoothConnectionRequest;

extern void bthInitializeConnectionRequest (BluetoothConnectionRequest *request);
extern BluetoothConnection *bthOpenConnection (const BluetoothConnectionRequest *request);
extern void bthCloseConnection (BluetoothConnection *connection);

extern int bthAwaitInput (BluetoothConnection *connection, int milliseconds);
extern ssize_t bthReadData (
  BluetoothConnection *connection, void *buffer, size_t size,
  int initialTimeout, int subsequentTimeout
);

extern ssize_t bthWriteData (BluetoothConnection *conection, const void *buffer, size_t size);

extern int isBluetoothDevice (const char **identifier);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_IO_BLUETOOTH */

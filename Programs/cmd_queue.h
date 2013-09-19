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

#ifndef BRLTTY_INCLUDED_CMD_QUEUE
#define BRLTTY_INCLUDED_CMD_QUEUE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void startBrailleCommands (void);
extern void stopBrailleCommands (void);

typedef int CommandHandler (int command, void *data);
extern int pushCommandHandler (CommandHandler *handler, void *data);

extern int enqueueCommand (int command);
extern int enqueueKeyEvent (unsigned char set, unsigned char key, int press);

extern int enqueueKey (unsigned char set, unsigned char key);
extern int enqueueKeys (uint32_t bits, unsigned char set, unsigned char key);
extern int enqueueUpdatedKeys (uint32_t new, uint32_t *old, unsigned char set, unsigned char key);

extern int enqueueXtScanCode (
  unsigned char code, unsigned char escape,
  unsigned char set00, unsigned char setE0, unsigned char setE1
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_CMD_QUEUE */

/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2009 by The BRLTTY Developers.
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

#ifndef BRLTTY_INCLUDED_KEYDEFS
#define BRLTTY_INCLUDED_KEYDEFS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
  unsigned char key;
  unsigned char set;
} KeyCode;

#define KEYS_PER_SET 0X100

typedef enum {
  KCS_UNBOUND,
  KCS_MODIFIERS,
  KCS_COMMAND
} KeyCodesState;

typedef struct {
  const char *name;
  KeyCode code;
} KeyNameEntry;

#define KEY_NAME_TABLE(name) const KeyNameEntry name[]
#define KEY_NAME_ENTRY(keyValue,keyName)  {.code={.key=keyValue}, .name=keyName}
#define LAST_KEY_NAME_ENTRY {.name=NULL}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_KEYDEFS */
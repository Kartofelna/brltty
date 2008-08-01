/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2008 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#ifndef BRLTTY_INCLUDED_ATB
#define BRLTTY_INCLUDED_ATB

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct AttributesTableStruct AttributesTable;

extern AttributesTable *attributesTable;

extern AttributesTable *compileAttributesTable (const char *name);
extern void destroyAttributesTable (AttributesTable *table);

extern void fixAttributesTablePath (char **path);

extern unsigned char convertAttributesToDots (AttributesTable *table, unsigned char attributes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_ATB */
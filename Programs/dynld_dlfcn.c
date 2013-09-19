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

#include <dlfcn.h>

#include "log.h"
#include "dynld.h"

static void
clearError (void) {
  dlerror();
}

static int
logError (void) {
  const char *error = dlerror();
  if (!error) return 1;

  logMessage(LOG_ERR, "%s", error);
  return 0;
}

static inline int
getSharedObjectLoadFlags (void) {
  int flags = 0;

#ifdef DL_LAZY
  flags |= DL_LAZY;
#else /* DL_LAZY */
  flags |= RTLD_LAZY | RTLD_GLOBAL;
#endif /* DL_LAZY */

  return flags;
}

void *
loadSharedObject (const char *path) {
  void *object;

  clearError();
  object = dlopen(path, getSharedObjectLoadFlags());
  if (!object) logError();
  return object;
}

void 
unloadSharedObject (void *object) {
  clearError();
  if (dlclose(object)) logError();
}

int 
findSharedSymbol (void *object, const char *symbol, void *pointerAddress) {
  void **address = pointerAddress;

  clearError(); /* clear any previous error condition */
  *address = dlsym(object, symbol);
  return logError();
}

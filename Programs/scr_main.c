/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2005 by The BRLTTY Team. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "misc.h"
#include "scr.h"
#include "scr_main.h"

static int
prepare_MainScreen (char **parameters) {
  return 1;
}

static int
open_MainScreen (void) {
  return 1;
}

static int
setup_MainScreen (void) {
  return 1;
}

static void
close_MainScreen (void) {
}

static int
uservt_MainScreen (int number) {
  return 1 + number;
}

void
initializeMainScreen (MainScreen *main) {
  initializeBaseScreen(&main->base);
  main->prepare = prepare_MainScreen;
  main->open = open_MainScreen;
  main->setup = setup_MainScreen;
  main->close = close_MainScreen;
  main->uservt = uservt_MainScreen;
}

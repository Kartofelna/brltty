/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2003 by The BRLTTY Team. All rights reserved.
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

/* spktest.c - Test progrm for the speech synthesizer drivers.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "options.h"
#include "spk.h"
#include "misc.h"

BEGIN_OPTION_TABLE
  {'t', "text-string"   , "string"   , NULL, 0,
   "Text to be spoken."},
  {'D', "data-directory", "directory", NULL, 0,
   "Path to directory for configuration files."},
  {'L', "library-directory", "directory", NULL, 0,
   "Path to directory for loading drivers."},
END_OPTION_TABLE

static const char *opt_textString = NULL;
static const char *opt_libraryDirectory = LIBRARY_DIRECTORY;
static const char *opt_dataDirectory = DATA_DIRECTORY;

static int
handleOption (const int option) {
  switch (option) {
    default:
      return 0;
    case 't':
      opt_textString = optarg;
      break;
    case 'D':
      opt_dataDirectory = optarg;
      break;
    case 'L':
      opt_libraryDirectory = optarg;
      break;
  }
  return 1;
}

static void
sayString (const char *string) {
  speech->say(string, strlen(string));
}

static int
sayLine (char *line, void *data) {
  sayString(line);
  return 1;
}

int
main (int argc, char *argv[]) {
  int status;
  const char *driver = NULL;

  processOptions(optionTable, optionCount, handleOption,
                 &argc, &argv, "[driver [parameter=value ...]]");

  if (argc) {
    driver = *argv++;
    --argc;
  }

  if ((speech = loadSpeechDriver(driver, opt_libraryDirectory))) {
    const char *const *parameterNames = speech->parameters;
    char **parameterSettings;
    if (!parameterNames) {
      static const char *const noNames[] = {NULL};
      parameterNames = noNames;
    }
    {
      const char *const *name = parameterNames;
      unsigned int count;
      char **setting;
      while (*name) ++name;
      count = name - parameterNames;
      if (!(parameterSettings = malloc((count + 1) * sizeof(*parameterSettings)))) {
        fprintf(stderr, "%s: Insufficient memory.\n", programName);
        exit(9);
      }
      setting = parameterSettings;
      while (count--) *setting++ = "";
      *setting = NULL;
    }
    while (argc) {
      char *assignment = *argv++;
      int ok = 0;
      char *delimiter = strchr(assignment, '=');
      if (!delimiter) {
        LogPrint(LOG_ERR, "Missing speech driver parameter value: %s", assignment);
      } else if (delimiter == assignment) {
        LogPrint(LOG_ERR, "Missing speech driver parameter name: %s", assignment);
      } else {
        size_t nameLength = delimiter - assignment;
        const char *const *name = parameterNames;
        while (*name) {
          if (strlen(*name) >= nameLength) {
            if (strncasecmp(*name, assignment, nameLength) == 0) {
              parameterSettings[name - parameterNames] = delimiter + 1;
              ok = 1;
              break;
            }
          }
          ++name;
        }
        if (!ok) LogPrint(LOG_ERR, "Invalid speech driver parameter: %s", assignment);
      }
      if (!ok) exit(2);
      --argc;
    }

    if (chdir(opt_dataDirectory) != -1) {
      speech->identify();		/* start-up messages */
      if (speech->open(parameterSettings)) {
        if (opt_textString) {
          sayString(opt_textString);
        } else {
          processLines(stdin, sayLine, NULL);
        }
        speech->close();		/* finish with the display */
        status = 0;
      } else {
        LogPrint(LOG_ERR, "Can't initialize speech driver.");
        status = 5;
      }
    } else {
      LogPrint(LOG_ERR, "Can't change directory to '%s': %s",
               opt_dataDirectory, strerror(errno));
      status = 4;
    }
  } else {
    LogPrint(LOG_ERR, "Can't load speech driver.");
    status = 3;
  }
  return status;
}

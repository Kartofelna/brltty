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

#include "prologue.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "misc.h"
#include "async.h"
#include "message.h"
#include "charset.h"
#include "drivers.h"
#include "brl.h"
#include "tbl.h"
#include "brl.auto.h"
#include "cmd.h"

#define BRLSYMBOL noBraille
#define DRIVER_NAME NoBraille
#define DRIVER_CODE no
#define DRIVER_COMMENT "no braille support"
#define DRIVER_VERSION ""
#define DRIVER_DEVELOPERS ""
#define BRLHELP "/dev/null"
#include "brl_driver.h"

static int
brl_construct (BrailleDisplay *brl, char **parameters, const char *device) {
  return 1;
}

static void
brl_destruct (BrailleDisplay *brl) {
}

static int
brl_readCommand (BrailleDisplay *brl, BRL_DriverCommandContext context) {
  return EOF;
}

static int
brl_writeWindow (BrailleDisplay *brl, const wchar_t *characters) {
  return 1;
}

const BrailleDriver *braille = &noBraille;

/*
 * Output braille translation tables.
 * The files *.auto.h (the default tables) are generated at compile-time.
 */
TranslationTable textTable = {
  #include "text.auto.h"
};
TranslationTable untextTable;

TranslationTable attributesTable = {
  #include "attrib.auto.h"
};

void *contractionTable = NULL;

int
haveBrailleDriver (const char *code) {
  return haveDriver(code, BRAILLE_DRIVER_CODES, driverTable);
}

const char *
getDefaultBrailleDriver (void) {
  return getDefaultDriver(driverTable);
}

const BrailleDriver *
loadBrailleDriver (const char *code, void **driverObject, const char *driverDirectory) {
  return loadDriver(code, driverObject,
                    driverDirectory, driverTable,
                    "braille", 'b', "brl",
                    &noBraille, &noBraille.definition);
}

void
identifyBrailleDriver (const BrailleDriver *driver, int full) {
  identifyDriver("Braille", &driver->definition, full);
}

void
identifyBrailleDrivers (int full) {
  const DriverEntry *entry = driverTable;
  while (entry->address) {
    const BrailleDriver *driver = entry++->address;
    identifyBrailleDriver(driver, full);
  }
}

void
initializeBrailleDisplay (BrailleDisplay *brl) {
  brl->x = 80;
  brl->y = 1;
  brl->helpPage = 0;
  brl->buffer = NULL;
  brl->writeDelay = 0;
  brl->bufferResized = NULL;
  brl->dataDirectory = NULL;
  brl->touchEnabled = 0;
  brl->highlightWindow = 0;
  brl->data = NULL;
}

unsigned int
drainBrailleOutput (BrailleDisplay *brl, int minimumDelay) {
  int duration = brl->writeDelay + 1;
  if (duration < minimumDelay) duration = minimumDelay;
  brl->writeDelay = 0;
  asyncWait(duration);
  return duration;
}

int
writeBrailleWindow (BrailleDisplay *brl, const wchar_t *text) {
  brl->cursor = -1;

  {
    int i;
    /* Do Braille translation using text table. Six-dot mode is ignored
     * since case can be important, and blinking caps won't work. 
     */
    for (i=0; i<brl->x*brl->y; ++i) brl->buffer[i] = convertWcharToDots(textTable, text[i]);
  }
  return braille->writeWindow(brl, text);
}

int
writeBrailleText (BrailleDisplay *brl, const char *text, size_t length) {
  size_t size = brl->x * brl->y;
  wchar_t characters[size];

  if (length > size) length = size;
  {
    int i;
    for (i=0; i<length; ++i) {
      wint_t character = convertCharToWchar(text[i]);
      if (character == WEOF) character = WC_C('?');
      characters[i] = character;
    }
  }
  wmemset(&characters[length], WC_C(' '), size-length);

  return writeBrailleWindow(brl, characters);
}

int
writeBrailleString (BrailleDisplay *brl, const char *string) {
  return writeBrailleText(brl, string, strlen(string));
}

int
showBrailleString (BrailleDisplay *brl, const char *string, unsigned int duration) {
  int ok = writeBrailleString(brl, string);
  drainBrailleOutput(brl, duration);
  return ok;
}

int
clearStatusCells (BrailleDisplay *brl) {
  if (braille->writeStatus) {
    unsigned char cells[BRL_MAX_STATUS_CELL_COUNT];        /* status cell buffer */
    memset(cells, 0, sizeof(cells));
    if (!braille->writeStatus(brl, cells)) return 0;
  }
  return 1;
}

int
setStatusText (BrailleDisplay *brl, const char *text) {
  if (braille->writeStatus) {
    unsigned char cells[BRL_MAX_STATUS_CELL_COUNT];        /* status cell buffer */
    memset(cells, 0, sizeof(cells));

    {
      int i;
      for (i=0; i<sizeof(cells); ++i) {
        char c;
        wchar_t wc;

        if (!(c = text[i])) break;
        if ((wc = convertCharToWchar(c)) == WEOF) wc = WC_C('?');
        cells[i] = convertWcharToDots(textTable, wc);
      }
    }

    if (!braille->writeStatus(brl, cells)) return 0;
  }
  return 1;
}

static void
brailleBufferResized (BrailleDisplay *brl, int infoLevel) {
  memset(brl->buffer, 0, brl->x*brl->y);
  LogPrint(infoLevel, "Braille Display Dimensions: %d %s, %d %s",
           brl->y, (brl->y == 1)? "row": "rows",
           brl->x, (brl->x == 1)? "column": "columns");
  if (brl->bufferResized) brl->bufferResized(infoLevel, brl->y, brl->x);
}

static int
resizeBrailleBuffer (BrailleDisplay *brl, int infoLevel) {
  if (brl->resizeRequired) {
    brl->resizeRequired = 0;

    if (brl->isCoreBuffer) {
      static void *currentAddress = NULL;
      static size_t currentSize = 0;
      size_t newSize = brl->x * brl->y;

      if (newSize > currentSize) {
        void *newAddress = malloc(newSize);

        if (!newAddress) {
          LogError("malloc");
          return 0;
        }

        if (currentAddress) free(currentAddress);
        currentAddress = newAddress;
        currentSize = newSize;
      }

      brl->buffer = currentAddress;
    }

    brailleBufferResized(brl, infoLevel);
  }

  return 1;
}

int
ensureBrailleBuffer (BrailleDisplay *brl, int infoLevel) {
  if ((brl->isCoreBuffer = brl->resizeRequired = brl->buffer == NULL)) {
    if (!resizeBrailleBuffer(brl, infoLevel)) return 0;
  } else {
    brailleBufferResized(brl, infoLevel);
  }
  return 1;
}

int
readBrailleCommand (BrailleDisplay *brl, BRL_DriverCommandContext context) {
  int command = braille->readCommand(brl, context);
  resizeBrailleBuffer(brl, LOG_INFO);
  return command;
}

#ifdef ENABLE_LEARN_MODE
int
learnMode (BrailleDisplay *brl, int poll, int timeout) {
  if (!setStatusText(brl, "lrn")) return 0;
  if (!message("command learn mode", MSG_NODELAY)) return 0;

  hasTimedOut(0);
  do {
    int command = readBrailleCommand(brl, BRL_CTX_SCREEN);
    if (command != EOF) {
      LogPrint(LOG_DEBUG, "Learn: command=%06X", command);
      if (IS_DELAYED_COMMAND(command)) continue;

      {
        int cmd = command & BRL_MSK_CMD;
        if (cmd == BRL_CMD_NOOP) continue;
        if (cmd == BRL_CMD_LEARN) return 1;
      }

      {
        char buffer[0X100];
        describeCommand(command, buffer, sizeof(buffer));
        LogPrint(LOG_DEBUG, "Learn: %s", buffer);
        if (!message(buffer, MSG_NODELAY|MSG_SILENT)) return 0;
      }

      hasTimedOut(0);
    }

    drainBrailleOutput(brl, poll);
  } while (!hasTimedOut(timeout));

  return message("done", 0);
}
#endif /* ENABLE_LEARN_MODE */

void
makeUntextTable (void) {
  reverseTranslationTable(textTable, untextTable);
}

void
makeOutputTable (const DotsTable dots, TranslationTable table) {
  static const DotsTable internalDots = {BRL_DOT1, BRL_DOT2, BRL_DOT3, BRL_DOT4, BRL_DOT5, BRL_DOT6, BRL_DOT7, BRL_DOT8};
  int byte, dot;
  memset(table, 0, sizeof(TranslationTable));
  for (byte=0; byte<TRANSLATION_TABLE_SIZE; byte++)
    for (dot=0; dot<DOTS_TABLE_SIZE; dot++)
      if (byte & internalDots[dot])
        table[byte] |= dots[dot];
}

/* Functions which support vertical and horizontal status cells. */

unsigned char
lowerDigit (unsigned char upper) {
  unsigned char lower = 0;
  if (upper & BRL_DOT1) lower |= BRL_DOT3;
  if (upper & BRL_DOT2) lower |= BRL_DOT7;
  if (upper & BRL_DOT4) lower |= BRL_DOT6;
  if (upper & BRL_DOT5) lower |= BRL_DOT8;
  return lower;
}

/* Dots for landscape (counterclockwise-rotated) digits. */
const unsigned char landscapeDigits[11] = {
  BRL_DOT1+BRL_DOT5+BRL_DOT2, BRL_DOT4,
  BRL_DOT4+BRL_DOT1,          BRL_DOT4+BRL_DOT5,
  BRL_DOT4+BRL_DOT5+BRL_DOT2, BRL_DOT4+BRL_DOT2,
  BRL_DOT4+BRL_DOT1+BRL_DOT5, BRL_DOT4+BRL_DOT1+BRL_DOT5+BRL_DOT2,
  BRL_DOT4+BRL_DOT1+BRL_DOT2, BRL_DOT1+BRL_DOT5,
  BRL_DOT1+BRL_DOT2+BRL_DOT4+BRL_DOT5
};

/* Format landscape representation of numbers 0 through 99. */
int
landscapeNumber (int x) {
  return landscapeDigits[(x / 10) % 10] | lowerDigit(landscapeDigits[x % 10]);  
}

/* Format landscape flag state indicator. */
int
landscapeFlag (int number, int on) {
  int dots = landscapeDigits[number % 10];
  if (on) dots |= lowerDigit(landscapeDigits[10]);
  return dots;
}

/* Dots for seascape (clockwise-rotated) digits. */
const unsigned char seascapeDigits[11] = {
  BRL_DOT5+BRL_DOT1+BRL_DOT4, BRL_DOT2,
  BRL_DOT2+BRL_DOT5,          BRL_DOT2+BRL_DOT1,
  BRL_DOT2+BRL_DOT1+BRL_DOT4, BRL_DOT2+BRL_DOT4,
  BRL_DOT2+BRL_DOT5+BRL_DOT1, BRL_DOT2+BRL_DOT5+BRL_DOT1+BRL_DOT4,
  BRL_DOT2+BRL_DOT5+BRL_DOT4, BRL_DOT5+BRL_DOT1,
  BRL_DOT1+BRL_DOT2+BRL_DOT4+BRL_DOT5
};

/* Format seascape representation of numbers 0 through 99. */
int
seascapeNumber (int x) {
  return lowerDigit(seascapeDigits[(x / 10) % 10]) | seascapeDigits[x % 10];  
}

/* Format seascape flag state indicator. */
int
seascapeFlag (int number, int on) {
  int dots = lowerDigit(seascapeDigits[number % 10]);
  if (on) dots |= seascapeDigits[10];
  return dots;
}

/* Dots for portrait digits - 2 numbers in one cells */
const unsigned char portraitDigits[11] = {
  BRL_DOT2+BRL_DOT4+BRL_DOT5, BRL_DOT1,
  BRL_DOT1+BRL_DOT2,          BRL_DOT1+BRL_DOT4,
  BRL_DOT1+BRL_DOT4+BRL_DOT5, BRL_DOT1+BRL_DOT5,
  BRL_DOT1+BRL_DOT2+BRL_DOT4, BRL_DOT1+BRL_DOT2+BRL_DOT4+BRL_DOT5,
  BRL_DOT1+BRL_DOT2+BRL_DOT5, BRL_DOT2+BRL_DOT4,
  BRL_DOT1+BRL_DOT2+BRL_DOT4+BRL_DOT5
};

/* Format portrait representation of numbers 0 through 99. */
int
portraitNumber (int x) {
  return portraitDigits[(x / 10) % 10] | lowerDigit(portraitDigits[x % 10]);  
}

/* Format portrait flag state indicator. */
int
portraitFlag (int number, int on) {
  int dots = lowerDigit(portraitDigits[number % 10]);
  if (on) dots |= portraitDigits[10];
  return dots;
}

void
setBrailleFirmness (BrailleDisplay *brl, int setting) {
  LogPrint(LOG_DEBUG, "setting braille firmness: %d", setting);
  braille->firmness(brl, setting);
}

void
setBrailleSensitivity (BrailleDisplay *brl, int setting) {
  LogPrint(LOG_DEBUG, "setting braille sensitivity: %d", setting);
  braille->sensitivity(brl, setting);
}

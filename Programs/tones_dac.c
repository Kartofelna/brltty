/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2002 by The BRLTTY Team. All rights reserved.
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
#include <string.h>
#include <errno.h>

#ifdef HAVE_FUNC_SIN
#include <math.h>
#endif /* HAVE_FUNC_SIN */

#include "misc.h"
#include "system.h"
#include "tones.h"

static int fileDescriptor = -1;
static int blockSize;
static int sampleRate;
static int channelCount;
static DacAmplitudeFormat amplitudeFormat;

static unsigned char *blockAddress = NULL;
static size_t blockUsed = 0;

static int openDac (void) {
   if (fileDescriptor != -1) return 1;
   if ((fileDescriptor = getDacDevice()) != -1) {
      setCloseOnExec(fileDescriptor);
      blockSize = getDacBlockSize(fileDescriptor);
      sampleRate = getDacSampleRate(fileDescriptor);
      channelCount = getDacChannelCount(fileDescriptor);
      amplitudeFormat = getDacAmplitudeFormat(fileDescriptor);
      if ((blockAddress = malloc(blockSize))) {
         blockUsed = 0;
         LogPrint(LOG_DEBUG, "DAC opened: fd=%d blk=%d rate=%d chan=%d fmt=%d",
                  fileDescriptor, blockSize, sampleRate, channelCount, amplitudeFormat);
         return 1;
      }
      close(fileDescriptor);
      fileDescriptor = -1;
   } else {
      LogPrint(LOG_WARNING, "Cannot open DAC.");
   }
   return 0;
}

static int flushBytes (void) {
   const unsigned char *address = blockAddress;
   size_t length = blockUsed;
   while (length > 0) {
      int written = write(fileDescriptor, address, length);
      if (written == -1) {
         if (errno != EAGAIN) {
            LogPrint(LOG_ERR, "Cannot write to DAC: %s", strerror(errno));
            return 0;
         }
         delay(10);
      } else {
         address += written;
         length -= written;
      }
   }
   blockUsed = 0;
   return 1;
}

static int writeBytes (const unsigned char *address, size_t length) {
   while (length > 0) {
      size_t count = blockSize - blockUsed;
      if (length < count) count = length;
      memcpy(&blockAddress[blockUsed], address, count);
      address += count;
      length -= count;
      if ((blockUsed += count) == blockSize)
         if (!flushBytes())
            return 0;
   }
   return 1;
}

static int writeAmplitude (int amplitude) {
   unsigned char sample[4];
   size_t length;
   int channel;
   switch (amplitudeFormat) {
      default:
         length = 0;
         break;
      case DAC_FMT_U8:
         amplitude += 0X8000;
      case DAC_FMT_S8:
         sample[0] = amplitude >> 8;
         length = 1;
         break;
      case DAC_FMT_U16B:
         amplitude += 0X8000;
      case DAC_FMT_S16B:
         sample[0] = amplitude >> 8;
         sample[1] = amplitude;
         length = 2;
         break;
      case DAC_FMT_U16L:
         amplitude += 0X8000;
      case DAC_FMT_S16L:
         sample[0] = amplitude;
         sample[1] = amplitude >> 8;
         length = 2;
         break;
      case DAC_FMT_ULAW: {
         int negative = amplitude < 0;
         int exponent = 0X7;
         unsigned char value;
         if (negative) amplitude = -amplitude;
         while ((exponent > 0) && !(amplitude & 0X4000)) {
            amplitude <<= 1;
            --exponent;
         }
         value = (exponent << 4) | ((amplitude >> 10) & 0XF);
         if (negative) value |= 0X80;
         sample[0] = ~value;
         length = 1;
         break;
      }
   }
   for (channel=0; channel<channelCount; ++channel)
      if (!writeBytes(sample, length))
         return 0;
   return 1;
}

static int generateDac (int note, int duration) {
   if (fileDescriptor != -1) {
      unsigned long int sampleCount = sampleRate * duration / 1000;
      if (note) {
         double waveLength = (double)sampleRate / noteFrequencies[note];
         int sampleNumber = 0;
         int wasNegative = 1;
         LogPrint(LOG_DEBUG, "Tone: msec=%d smct=%lu note=%d wvln=%.2E",
                  duration, sampleCount, note, waveLength);
         while (1) {
            int amplitude =
#ifdef HAVE_FUNC_SIN
               (int)(sin(((double)sampleNumber / waveLength) * (2.0 * M_PI)) * 32767.0)
#else
               ((((int)((double)sampleNumber * 2.0 / waveLength) % 2) * -2) + 1) * 20000
#endif /* HAVE_FUNC_SIN */
               ;
            int isNegative = amplitude < 0;
            if ((sampleNumber >= sampleCount) && wasNegative && !isNegative) break;
            if (!writeAmplitude(amplitude)) return 0;
            wasNegative = isNegative;
            ++sampleNumber;
         }
      } else {
         LogPrint(LOG_DEBUG, "Tone: msec=%d smct=%lu note=%d",
                  duration, sampleCount, note);
         while (sampleCount) {
            if (!writeAmplitude(0)) return 0;
            --sampleCount;
         }
      }
      return 1;
   }
   return 0;
}

static int flushBlock (void) {
   while (blockUsed)
      if (!writeAmplitude(0))
         return 0;
   return 1;
}

static int flushDac (void) {
   return (fileDescriptor != -1)? flushBlock(): 0;
}

static void closeDac (void) {
   if (fileDescriptor != -1) {
      flushBlock();
      free(blockAddress);
      blockAddress = NULL;
      close(fileDescriptor);
      fileDescriptor = -1;
      LogPrint(LOG_DEBUG, "DAC closed.");
   }
}

const ToneGenerator dacToneGenerator = {
   openDac,
   generateDac,
   flushDac,
   closeDac
};

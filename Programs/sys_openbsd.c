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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/audioio.h>

#ifdef HAVE_FUNC_DLOPEN 
#  include <dlfcn.h>
#endif /* HAVE_FUNC_DLOPEN */

#include "misc.h"
#include "system.h"

char *
getBootParameters (void) {
  return NULL;
}

void *
loadSharedObject (const char *path) {
#ifdef HAVE_FUNC_DLOPEN 
  void *object = dlopen(path, DL_LAZY);
  if (object) return object;
  LogPrint(LOG_ERR, "%s", dlerror());
#endif /* HAVE_FUNC_DLOPEN */
  return NULL;
}

void 
unloadSharedObject (void *object) {
#ifdef HAVE_FUNC_DLOPEN 
  dlclose(object);
#endif /* HAVE_FUNC_DLOPEN */
}

int 
findSharedSymbol (void *object, const char *symbol, const void **address) {
#ifdef HAVE_FUNC_DLOPEN 
  const char *error;
  *address = dlsym(object, symbol);
  if (!(error = dlerror())) return 1;
  LogPrint(LOG_ERR, "%s", error);
#endif /* HAVE_FUNC_DLOPEN */
  return 0;
}

int
canBeep (void) {
  return 0;
}

int
timedBeep (unsigned short frequency, unsigned short milliseconds) {
  return 0;
}

int
startBeep (unsigned short frequency) {
  return 0;
}

int
stopBeep (void) {
  return 0;
}

#ifdef ENABLE_PCM_TUNES
int
getPcmDevice (int errorLevel) {
  int descriptor;
  const char *path = getenv("AUDIODEV");
  if (!path) path = "/dev/audio";
  if ((descriptor = open(path, O_WRONLY|O_NONBLOCK)) != -1) {
    audio_info_t info;
    AUDIO_INITINFO(&info);
    info.play.encoding = AUDIO_ENCODING_LINEAR;
    info.play.sample_rate = 16000;
    info.play.channels = 1;
    info.play.precision = 16;
    /* info.play.gain = AUDIO_MAX_GAIN; */
    ioctl(descriptor, AUDIO_SETINFO, &info);
  } else {
    LogPrint(errorLevel, "Cannot open PCM device: %s: %s", path, strerror(errno));
  }
  return descriptor;
}

int
getPcmBlockSize (int descriptor) {
  return 0X100;
}

int
getPcmSampleRate (int descriptor) {
  if (descriptor != -1) {
    audio_info_t info;
    if (ioctl(descriptor, AUDIO_GETINFO, &info) != -1) return info.play.sample_rate;
  }
  return 8000;
}

int
getPcmChannelCount (int descriptor) {
  if (descriptor != -1) {
    audio_info_t info;
    if (ioctl(descriptor, AUDIO_GETINFO, &info) != -1) return info.play.channels;
  }
  return 1;
}

PcmAmplitudeFormat
getPcmAmplitudeFormat (int descriptor) {
  if (descriptor != -1) {
    audio_info_t info;
    if (ioctl(descriptor, AUDIO_GETINFO, &info) != -1) {
      switch (info.play.encoding) {
	default:
	  break;
	case AUDIO_ENCODING_LINEAR:
	  if (info.play.precision == 8) return PCM_FMT_S8;
	  if (info.play.precision == 16) return PCM_FMT_S16B;
	  break;
	case AUDIO_ENCODING_ULAW:
	  return PCM_FMT_ULAW;
	case AUDIO_ENCODING_LINEAR8:
	  return PCM_FMT_U8;
      }
    }
  }
  return PCM_FMT_UNKNOWN;
}
#endif /* ENABLE_PCM_TUNES */

#ifdef ENABLE_MIDI_TUNES
int
getMidiDevice (int errorLevel, MidiBufferFlusher flushBuffer) {
  LogPrint(errorLevel, "MIDI device not supported.");
  return -1;
}

void
setMidiInstrument (unsigned char channel, unsigned char instrument) {
}

void
beginMidiBlock (int descriptor) {
}

void
endMidiBlock (int descriptor) {
}

void
startMidiNote (unsigned char channel, unsigned char note, unsigned char volume) {
}

void
stopMidiNote (unsigned char channel) {
}

void
insertMidiWait (int duration) {
}
#endif /* ENABLE_MIDI_TUNES */

int
enablePorts (int errorLevel, unsigned short int base, unsigned short int count) {
  LogPrint(errorLevel, "I/O ports not supported.");
  return 0;
}

int
disablePorts (unsigned short int base, unsigned short int count) {
  return 0;
}

unsigned char
readPort1 (unsigned short int port) {
  return 0;
}

void
writePort1 (unsigned short int port, unsigned char value) {
}

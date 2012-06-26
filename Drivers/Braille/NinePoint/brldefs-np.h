/*
 * libbrlapi - A library providing access to braille terminals for applications.
 *
 * Copyright (C) 2002-2012 Sébastien Hinderer <Sebastien.Hinderer@ens-lyon.org>
 *
 * libbrlapi comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU Lesser General Public License, as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version. Please see the file LICENSE-LGPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#ifndef BRLTTY_INCLUDED_NP_BRLDEFS
#define BRLTTY_INCLUDED_NP_BRLDEFS

#define NP_KEY_NAVIGATION_PRESS 0X20
#define NP_KEY_ROUTING_MIN 0X80
#define NP_KEY_ROUTING_MAX 0X87

typedef enum {
  NP_KEY_Brl1     = 0X41,
  NP_KEY_Brl2     = 0X42,
  NP_KEY_Brl3     = 0X43,
  NP_KEY_Brl4     = 0X44,
  NP_KEY_Brl5     = 0X45,
  NP_KEY_Brl6     = 0X46,
  NP_KEY_Brl7     = 0X47,
  NP_KEY_Brl8     = 0X48,
  NP_KEY_Up       = 0X49,
  NP_KEY_Enter    = 0X4A,
  NP_KEY_Right    = 0X4B,
  NP_KEY_Space    = 0X4C,
  NP_KEY_Menu     = 0X4D,
  NP_KEY_Down     = 0X51,
  NP_KEY_Left     = 0X53
} NP_NavigationKey;

typedef enum {
  NP_SET_NavigationKeys,
  NP_SET_RoutingKeys
} NP_KeySet;

#endif /* BRLTTY_INCLUDED_NP_BRLDEFS */ 

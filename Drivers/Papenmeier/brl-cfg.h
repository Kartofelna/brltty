/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2004 by The BRLTTY Team. All rights reserved.
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

/*
 * Support for all Papenmeier Terminal + config file
 *   Heimo.Sch�n <heimo.schoen@gmx.at>
 *   August H�randl <august.hoerandl@gmx.at>
 *
 * Papenmeier/brl-cfg.h
 *  some defines and the big config table
 */

 /* supported terminals:
   id   name                     help  x  y  stat frnt easy
    0, "BRAILLEX Compact 486", 	  '1', 40, 1,  0,  9,  0
    1, "BRAILLEX 2D Lite (plus)", '2', 40, 1, 13,  9,  0
    2, "BRAILLEX Compact/Tiny",	  '3', 40, 1,  0,  9,  0
    3, "BRAILLEX 2D Screen Soft", '4', 80, 1, 22, 13,  0
    6, "BRAILLEX IB 80 cr soft",  '5', 80, 1,  4,  9,  0
   64, "BRAILLEX EL 2D-40",	  '6', 40, 1, 13,  0,  1
   65, "BRAILLEX EL 2D-66",	  '7', 66, 1, 13,  0,  1
   66, "BRAILLEX EL 80 (IB)",	  '8', 80, 1,  2,  0,  1
   67, "BRAILLEX EL 2D-80",	  '8', 80, 1,  20, 0,  1
   68, "BRAILLEX EL 40 P",	  '8', 40, 1,  0,  0,  1
   69, "BRAILLEX Elba 32",	  '8', 32, 1,  0,  0,  1
   70, "BRAILLEX Elba 20",	  '8', 20, 1,  0,  0,  1
 */

#include <inttypes.h>

#include "Programs/brl.h"
typedef enum {
  CMD_INPUT = DriverCommandCount /* toggle input mode */,
  CMD_NODOTS = VAL_PASSDOTS /* input character corresponding to no braille dots */
} InternalDriverCommands;

typedef enum {
  STAT_INPUT = StatusCellCount /* input mode */,
  InternalStatusCellCount
} InternalStatusCell;

#define OFFS_FRONT      0
#define OFFS_STAT    1000
#define OFFS_ROUTE   2000  
#define OFFS_EASY    3000
#define OFFS_SWITCH  4000
#define OFFS_CR      5000

#define ROUTINGKEY      -9999  /* virtual routing key */
#define NOKEY           -1

#define KEYMAX 8

#define  STATMAX   22
#define  FRONTMAX  13
#define  EASYMAX   8
#define  NAMEMAX   80
#define  MODMAX    16
#define  CMDMAX    200
#define  INPUTSPECMAX 20
#define  HELPLEN   80

#define OFFS_EMPTY       0
#define OFFS_HORIZ    1000	/* added to status show */
#define OFFS_FLAG     2000
#define OFFS_NUMBER   3000

/* easy bar - code as delivered by the terminals */
#define EASY_L1 1
#define EASY_L2 2    
#define EASY_U1 3 
#define EASY_U2 4
#define EASY_R1 5 
#define EASY_R2 6  
#define EASY_D1 7 
#define EASY_D2 8  

#define SWITCH_LEFT_REAR   1
#define SWITCH_LEFT_FRONT  2
#define KEY_LEFT_REAR      3
#define KEY_LEFT_FRONT     4
#define KEY_RIGHT_REAR     5
#define KEY_RIGHT_FRONT    6
#define SWITCH_RIGHT_REAR  7
#define SWITCH_RIGHT_FRONT 8

typedef struct {
  int code;		/* the code to sent */
  int16_t keycode;	/* the key to press */
  uint16_t modifiers;	/* modifiers (bitfield) */
} commands;

typedef struct {
  unsigned char ident;		/* identity of terminal */

#ifdef ENABLE_PM_CONFIGURATION_FILE
  char name[NAMEMAX];		/* name of terminal */
  char helpfile[HELPLEN];	/* filename of local helpfile */
#else /* ENABLE_PM_CONFIGURATION_FILE */
  const char *name;		/* name of terminal */
  const char *helpfile;		/* filename of local helpfile */
#endif /* ENABLE_PM_CONFIGURATION_FILE */

  int x, y;			/* size of display */
  int statcells;		/* number of status cells */
  int frontkeys;		/* number of frontkeys */
  int haseasybar;		/* terminal has an easy bar */

  int statshow[STATMAX];	/* status cells: info to show */
  int modifiers[MODMAX];	/* keys used as modifier */
  commands cmds[CMDMAX];

} one_terminal; 

/* some macros for terminals with the same layout -
 * named after there usage
 */

#define CHGONOFF(cmd, offs, on, off) \
      { cmd                 , offs, 0  }, \
      { cmd | VAL_TOGGLE_ON , offs, on }, \
      { cmd | VAL_TOGGLE_OFF, offs, off}


/* modifiers for 9 front keys */
#define MOD_FRONT_9 \
     OFFS_FRONT + 1, \
     OFFS_FRONT + 9, \
     OFFS_FRONT + 2, \
     OFFS_FRONT + 8

/* commands for 9 front keys */
#define CMDS_FRONT_9 \
     { CMD_FWINLT     , NOKEY         , 0X1 }, \
     { CMD_FWINRT     , NOKEY         , 0X2 }, \
     { CMD_HWINLT     , NOKEY         , 0X4 }, \
     { CMD_HWINRT     , NOKEY         , 0X8 }, \
                                               \
     { CMD_HOME       , OFFS_FRONT + 5, 0X0 }, \
     { CMD_LNBEG      , OFFS_FRONT + 5, 0X1 }, \
     { CMD_LNEND      , OFFS_FRONT + 5, 0X2 }, \
     { CMD_CHRLT      , OFFS_FRONT + 5, 0X4 }, \
     { CMD_CHRRT      , OFFS_FRONT + 5, 0X8 }, \
                                               \
     { CMD_WINUP      , OFFS_FRONT + 4, 0X0 }, \
     { CMD_PRDIFLN    , OFFS_FRONT + 4, 0X1 }, \
     { CMD_ATTRUP     , OFFS_FRONT + 4, 0X2 }, \
     { CMD_PRPGRPH    , OFFS_FRONT + 4, 0X4 }, \
     { CMD_PRSEARCH   , OFFS_FRONT + 4, 0X8 }, \
                                               \
     { CMD_WINDN      , OFFS_FRONT + 6, 0X0 }, \
     { CMD_NXDIFLN    , OFFS_FRONT + 6, 0X1 }, \
     { CMD_ATTRDN     , OFFS_FRONT + 6, 0X2 }, \
     { CMD_NXPGRPH    , OFFS_FRONT + 6, 0X4 }, \
     { CMD_NXSEARCH   , OFFS_FRONT + 6, 0X8 }, \
                                               \
     { CMD_LNUP       , OFFS_FRONT + 3, 0X0 }, \
     { CMD_TOP_LEFT   , OFFS_FRONT + 3, 0X1 }, \
     { CMD_TOP        , OFFS_FRONT + 3, 0X2 }, \
                                               \
     { CMD_LNDN       , OFFS_FRONT + 7, 0X0 }, \
     { CMD_BOT_LEFT   , OFFS_FRONT + 7, 0X1 }, \
     { CMD_BOT        , OFFS_FRONT + 7, 0X2 }, \
                                               \
     { CR_ROUTE       , ROUTINGKEY    , 0X0 }, \
     { CR_CUTBEGIN    , ROUTINGKEY    , 0X1 }, \
     { CR_CUTRECT     , ROUTINGKEY    , 0X2 }, \
     { CR_PRINDENT    , ROUTINGKEY    , 0X4 }, \
     { CR_NXINDENT    , ROUTINGKEY    , 0X8 }, \
     { CMD_PASTE      , NOKEY         , 0X3 }


/* modifiers for 13 front keys */
#define MOD_FRONT_13  \
     OFFS_FRONT +  4, \
     OFFS_FRONT +  3, \
     OFFS_FRONT +  2, \
     OFFS_FRONT + 10, \
     OFFS_FRONT + 11, \
     OFFS_FRONT + 12, \
     OFFS_FRONT +  1, \
     OFFS_FRONT + 13

/* commands for 13 front keys */
#define CMDS_FRONT_13 \
      { CMD_HOME                    , OFFS_FRONT +  7, 0000 }, \
      { CMD_TOP                     , OFFS_FRONT +  6, 0000 }, \
      { CMD_BOT                     , OFFS_FRONT +  8, 0000 }, \
      { CMD_LNUP                    , OFFS_FRONT +  5, 0000 }, \
      { CMD_LNDN                    , OFFS_FRONT +  9, 0000 }, \
                                                               \
      { CMD_PRDIFLN                 , NOKEY          , 0001 }, \
      { CMD_NXDIFLN                 , NOKEY          , 0010 }, \
      { CMD_WINUP                   , NOKEY          , 0002 }, \
      { CMD_WINDN                   , NOKEY          , 0020 }, \
      { CMD_ATTRUP                  , NOKEY          , 0004 },         \
      { CMD_ATTRDN                  , NOKEY          , 0040 },        \
      { CMD_PRPROMPT                , NOKEY          , 0100 },        \
      { CMD_NXPROMPT                , NOKEY          , 0200 },        \
                                                               \
      { CMD_PRPGRPH                 , NOKEY          , 0003 },        \
      { CMD_NXPGRPH                 , NOKEY          , 0030 },        \
      { CMD_PRSEARCH                , NOKEY          , 0104 },        \
      { CMD_NXSEARCH                , NOKEY          , 0240 },        \
      { CR_PRINDENT                 , ROUTINGKEY     , 0104 },        \
      { CR_NXINDENT                 , ROUTINGKEY     , 0240 },        \
                                                               \
      { CMD_LNBEG                   , OFFS_FRONT +  7, 0001 }, \
      { CMD_TOP_LEFT                , OFFS_FRONT +  6, 0001 }, \
      { CMD_BOT_LEFT                , OFFS_FRONT +  8, 0001 }, \
      { CMD_FWINLT                  , OFFS_FRONT +  5, 0001 }, \
      { CMD_FWINRT                  , OFFS_FRONT +  9, 0001 }, \
      { CR_DESCCHAR                 , ROUTINGKEY     , 0001 }, \
                                                               \
      { CMD_LNEND                   , OFFS_FRONT +  7, 0010 }, \
      { CMD_CHRLT                   , OFFS_FRONT +  6, 0010 }, \
      { CMD_CHRRT                   , OFFS_FRONT +  8, 0010 }, \
      { CMD_HWINLT                  , OFFS_FRONT +  5, 0010 }, \
      { CMD_HWINRT                  , OFFS_FRONT +  9, 0010 }, \
      { CR_SETLEFT                  , ROUTINGKEY     , 0010 }, \
                                                               \
      { VAL_PASSKEY+VPK_INSERT      , OFFS_FRONT +  7, 0002 }, \
      { VAL_PASSKEY+VPK_PAGE_UP     , OFFS_FRONT +  6, 0002 }, \
      { VAL_PASSKEY+VPK_PAGE_DOWN   , OFFS_FRONT +  8, 0002 }, \
      { VAL_PASSKEY+VPK_CURSOR_UP   , OFFS_FRONT +  5, 0002 }, \
      { VAL_PASSKEY+VPK_CURSOR_DOWN , OFFS_FRONT +  9, 0002 }, \
      { CR_SWITCHVT                 , ROUTINGKEY     , 0002 }, \
                                                               \
      { VAL_PASSKEY+VPK_DELETE      , OFFS_FRONT +  7, 0020 }, \
      { VAL_PASSKEY+VPK_HOME        , OFFS_FRONT +  6, 0020 }, \
      { VAL_PASSKEY+VPK_END         , OFFS_FRONT +  8, 0020 }, \
      { VAL_PASSKEY+VPK_CURSOR_LEFT , OFFS_FRONT +  5, 0020 }, \
      { VAL_PASSKEY+VPK_CURSOR_RIGHT, OFFS_FRONT +  9, 0020 }, \
      { VAL_PASSKEY+VPK_FUNCTION    , ROUTINGKEY     , 0020 }, \
                                                               \
      { CMD_NODOTS                  , OFFS_FRONT +  7, 0004 }, \
      { VAL_PASSKEY+VPK_ESCAPE      , OFFS_FRONT +  6, 0004 }, \
      { VAL_PASSKEY+VPK_TAB         , OFFS_FRONT +  8, 0004 }, \
      { VAL_PASSKEY+VPK_BACKSPACE   , OFFS_FRONT +  5, 0004 }, \
      { VAL_PASSKEY+VPK_RETURN      , OFFS_FRONT +  9, 0004 }, \
                                                               \
      { CMD_SPKHOME                 , OFFS_FRONT +  7, 0040 }, \
      { CMD_SAY_ABOVE               , OFFS_FRONT +  6, 0040 }, \
      { CMD_SAY_BELOW               , OFFS_FRONT +  8, 0040 }, \
      { CMD_MUTE                    , OFFS_FRONT +  5, 0040 }, \
      { CMD_SAY_LINE                , OFFS_FRONT +  9, 0040 }, \
                                                               \
      { CR_CUTBEGIN                 , ROUTINGKEY     , 0100 }, \
      { CR_CUTAPPEND                , ROUTINGKEY     , 0004 }, \
      { CR_CUTLINE                  , ROUTINGKEY     , 0040 }, \
      { CR_CUTRECT                  , ROUTINGKEY     , 0200 }, \
                                                               \
      { CR_ROUTE                    , ROUTINGKEY     , 0000 }
	

/* modifiers for switches */
#define MOD_EASY \
     OFFS_SWITCH + SWITCH_LEFT_REAR  , \
     OFFS_SWITCH + SWITCH_LEFT_FRONT , \
     OFFS_SWITCH + SWITCH_RIGHT_REAR , \
     OFFS_SWITCH + SWITCH_RIGHT_FRONT, \
     OFFS_SWITCH + KEY_LEFT_REAR     , \
     OFFS_SWITCH + KEY_LEFT_FRONT    , \
     OFFS_SWITCH + KEY_RIGHT_REAR    , \
     OFFS_SWITCH + KEY_RIGHT_FRONT   , \
     OFFS_EASY   + EASY_U1,            \
     OFFS_EASY   + EASY_D1,            \
     OFFS_EASY   + EASY_L1,            \
     OFFS_EASY   + EASY_R1,            \
     OFFS_EASY   + EASY_U2,            \
     OFFS_EASY   + EASY_D2,            \
     OFFS_EASY   + EASY_L2,            \
     OFFS_EASY   + EASY_R2

/* commands for easy bar + switches */
#define CMDS_EASY_X(dir, key, swt, cmd1, cmd2) \
  {cmd1, key, swt|(dir<<0X8)}, \
  {cmd2, key, swt|(dir<<0XC)}
#define CMDS_EASY_U(key, swt, cmd1, cmd2) CMDS_EASY_X(0X1, key, swt, cmd1, cmd2)
#define CMDS_EASY_D(key, swt, cmd1, cmd2) CMDS_EASY_X(0X2, key, swt, cmd1, cmd2)
#define CMDS_EASY_L(key, swt, cmd1, cmd2) CMDS_EASY_X(0X4, key, swt, cmd1, cmd2)
#define CMDS_EASY_R(key, swt, cmd1, cmd2) CMDS_EASY_X(0X8, key, swt, cmd1, cmd2)
#define CMDS_EASY_K(key, swt, u1, d1, u2, d2, l1, r1, l2, r2) \
  CMDS_EASY_U(key, swt, u1, u2), \
  CMDS_EASY_D(key, swt, d1, d2), \
  CMDS_EASY_L(key, swt, l1, l2), \
  CMDS_EASY_R(key, swt, r1, r2)
#define CMDS_EASY_C(swt) \
  {CMD_PREFMENU    , NOKEY, 0X10|swt}, \
  {CMD_INFO        , NOKEY, 0X20|swt}, \
  {CMD_HELP        , NOKEY, 0X40|swt}, \
  {CMD_LEARN       , NOKEY, 0X80|swt}, \
  {CMD_PASTE       , NOKEY, 0X50|swt}, \
  {CMD_CSRJMP_VERT , NOKEY, 0X90|swt}, \
  {CMD_BACK        , NOKEY, 0X60|swt}, \
  {CMD_HOME        , NOKEY, 0XA0|swt}, \
  {CR_ROUTE        , ROUTINGKEY, swt}, \
  CMDS_EASY_K(ROUTINGKEY, swt, \
              CR_PRINDENT, CR_NXINDENT, CR_SETLEFT, CR_DESCCHAR, \
              CR_CUTAPPEND, CR_CUTLINE, CR_CUTBEGIN, CR_CUTRECT)
#define CMDS_EASY \
  CMDS_EASY_K(NOKEY, 0X00, \
              CMD_LNUP, CMD_LNDN, CMD_TOP, CMD_BOT, \
              CMD_FWINLT, CMD_FWINRT, CMD_LNBEG, CMD_LNEND), \
  CMDS_EASY_C(0X00), \
  CMDS_EASY_K(NOKEY, 0X01, \
              CMD_PRDIFLN, CMD_NXDIFLN, CMD_ATTRUP, CMD_ATTRDN, \
              CMD_PRPROMPT, CMD_NXPROMPT, CMD_PRPGRPH, CMD_NXPGRPH), \
  CMDS_EASY_C(0X01), \
  CMDS_EASY_K(NOKEY, 0X04, \
              VAL_PASSKEY+VPK_CURSOR_UP, VAL_PASSKEY+VPK_CURSOR_DOWN, VAL_PASSKEY+VPK_PAGE_UP, VAL_PASSKEY+VPK_PAGE_DOWN, \
              CMD_FWINLT, CMD_FWINRT, CMD_LNBEG, CMD_LNEND), \
  CMDS_EASY_C(0X04), \
  CMDS_EASY_K(NOKEY, 0X05, \
              VAL_PASSKEY+VPK_CURSOR_UP, VAL_PASSKEY+VPK_CURSOR_DOWN, VAL_PASSKEY+VPK_PAGE_UP, VAL_PASSKEY+VPK_PAGE_DOWN, \
              VAL_PASSKEY+VPK_CURSOR_LEFT, VAL_PASSKEY+VPK_CURSOR_RIGHT, VAL_PASSKEY+VPK_HOME, VAL_PASSKEY+VPK_END), \
  CMDS_EASY_C(0X05), \
  CMDS_EASY_K(NOKEY, 0X02, \
              CMD_PRSEARCH, CMD_NXSEARCH, CMD_TOP_LEFT, CMD_BOT_LEFT, \
              CMD_CHRLT, CMD_CHRRT, CMD_HWINLT, CMD_HWINRT), \
  CMDS_EASY_C(0X02), \
  CMDS_EASY_K(NOKEY, 0X08, \
              CMD_SIXDOTS, CMD_SKPIDLNS, CMD_FREEZE, CMD_SKPBLNKWINS, \
              CMD_ATTRVIS, CMD_CSRVIS, CMD_DISPMD, CMD_CSRTRK), \
  CMDS_EASY_C(0X08)


/* what to show for 2 status cells */
#define SHOW_STAT_2 \
      OFFS_NUMBER + STAT_BRLROW, \
      OFFS_NUMBER + STAT_CSRROW

/* commands for 2 status keys */
#define CMDS_STAT_2 \
      { CMD_HELP , OFFS_STAT + 1, 0 }, \
      { CMD_LEARN, OFFS_STAT + 2, 0 }


/* what to show for 4 status cells */
#define SHOW_STAT_4 \
      OFFS_NUMBER + STAT_BRLROW, \
      OFFS_NUMBER + STAT_CSRROW, \
      OFFS_NUMBER + STAT_CSRCOL, \
      OFFS_FLAG   + STAT_DISPMD

/* commands for 4 status keys */
#define CMDS_STAT_4 \
      { CMD_HELP       , OFFS_STAT + 1, 0 }, \
      { CMD_LEARN      , OFFS_STAT + 2, 0 }, \
      { CMD_CSRJMP_VERT, OFFS_STAT + 3, 0 }, \
      { CMD_DISPMD     , OFFS_STAT + 4, 0 }


/* what to show for 13 status cells */
#define SHOW_STAT_13 \
      OFFS_HORIZ + STAT_BRLROW  , \
      OFFS_EMPTY                , \
      OFFS_HORIZ + STAT_CSRROW  , \
      OFFS_HORIZ + STAT_CSRCOL  , \
      OFFS_EMPTY                , \
      OFFS_FLAG  + STAT_CSRTRK  , \
      OFFS_FLAG  + STAT_DISPMD  , \
      OFFS_FLAG  + STAT_FREEZE  , \
      OFFS_EMPTY                , \
      OFFS_EMPTY                , \
      OFFS_FLAG  + STAT_CSRVIS  , \
      OFFS_FLAG  + STAT_ATTRVIS , \
      OFFS_EMPTY

/* commands for 13 status keys */
#define CMDS_STAT_13 \
      CHGONOFF( CMD_HELP       , OFFS_STAT +  1, 2, 1), \
              { CMD_LEARN      , OFFS_STAT +  2, 0   }, \
              { CMD_CSRJMP_VERT, OFFS_STAT +  3, 0   }, \
              { CMD_BACK       , OFFS_STAT +  4, 0   }, \
      CHGONOFF( CMD_INFO       , OFFS_STAT +  5, 2, 1), \
      CHGONOFF( CMD_CSRTRK     , OFFS_STAT +  6, 2, 1), \
      CHGONOFF( CMD_DISPMD     , OFFS_STAT +  7, 2, 1), \
      CHGONOFF( CMD_FREEZE     , OFFS_STAT +  8, 2, 1), \
              { CMD_PREFMENU   , OFFS_STAT +  9, 0   }, \
              { CMD_PREFLOAD   , OFFS_STAT + 10, 0   }, \
      CHGONOFF( CMD_CSRVIS     , OFFS_STAT + 11, 2, 1), \
      CHGONOFF( CMD_ATTRVIS    , OFFS_STAT + 12, 2, 1), \
              { CMD_PASTE      , OFFS_STAT + 13, 0   }


/* what to show for 20 status cells */
#define SHOW_STAT_20 \
      OFFS_HORIZ + STAT_BRLROW    , \
      OFFS_EMPTY                  , \
      OFFS_HORIZ + STAT_CSRROW    , \
      OFFS_HORIZ + STAT_CSRCOL    , \
      OFFS_EMPTY                  , \
      OFFS_FLAG  + STAT_CSRTRK    , \
      OFFS_FLAG  + STAT_DISPMD    , \
      OFFS_FLAG  + STAT_FREEZE    , \
      OFFS_EMPTY                  , \
      OFFS_HORIZ + STAT_SCRNUM    , \
      OFFS_EMPTY                  , \
      OFFS_FLAG  + STAT_CSRVIS    , \
      OFFS_FLAG  + STAT_ATTRVIS   , \
      OFFS_FLAG  + STAT_CAPBLINK  , \
      OFFS_FLAG  + STAT_SIXDOTS   , \
      OFFS_FLAG  + STAT_SKPIDLNS  , \
      OFFS_FLAG  + STAT_TUNES     , \
      OFFS_FLAG  + STAT_AUTOSPEAK , \
      OFFS_FLAG  + STAT_AUTOREPEAT, \
      OFFS_EMPTY

/* commands for 20 status keys */
#define CMDS_STAT_20 \
              { CMD_HELP       , OFFS_STAT +  1, 0000       }, \
              { CMD_LEARN      , OFFS_STAT +  2, 0000       }, \
              { CMD_CSRJMP_VERT, OFFS_STAT +  3, 0000       }, \
              { CMD_BACK       , OFFS_STAT +  4, 0000       }, \
              { CMD_INFO       , OFFS_STAT +  5, 0000       }, \
      CHGONOFF( CMD_CSRTRK     , OFFS_STAT +  6, 0200, 0100 ), \
      CHGONOFF( CMD_DISPMD     , OFFS_STAT +  7, 0200, 0100 ), \
      CHGONOFF( CMD_FREEZE     , OFFS_STAT +  8, 0200, 0100 ), \
              { CMD_PREFMENU   , OFFS_STAT +  9, 0000       }, \
              { CMD_PREFSAVE   , OFFS_STAT + 10, 0000       }, \
              { CMD_PREFLOAD   , OFFS_STAT + 11, 0000       }, \
      CHGONOFF( CMD_CSRVIS     , OFFS_STAT + 12, 0200, 0100 ), \
      CHGONOFF( CMD_ATTRVIS    , OFFS_STAT + 13, 0200, 0100 ), \
      CHGONOFF( CMD_CAPBLINK   , OFFS_STAT + 14, 0200, 0100 ), \
      CHGONOFF( CMD_SIXDOTS    , OFFS_STAT + 15, 0200, 0100 ), \
      CHGONOFF( CMD_SKPIDLNS   , OFFS_STAT + 16, 0200, 0100 ), \
      CHGONOFF( CMD_TUNES      , OFFS_STAT + 17, 0200, 0100 ), \
      CHGONOFF( CMD_AUTOSPEAK  , OFFS_STAT + 18, 0200, 0100 ), \
      CHGONOFF( CMD_AUTOREPEAT , OFFS_STAT + 19, 0200, 0100 ), \
      CHGONOFF( CMD_PASTE      , OFFS_STAT + 20, 0200, 0100 )


/* what to show for 22 status cells */
#define SHOW_STAT_22 \
      OFFS_HORIZ + STAT_BRLROW    , \
      OFFS_EMPTY                  , \
      OFFS_HORIZ + STAT_CSRROW    , \
      OFFS_HORIZ + STAT_CSRCOL    , \
      OFFS_EMPTY                  , \
      OFFS_FLAG  + STAT_CSRTRK    , \
      OFFS_FLAG  + STAT_DISPMD    , \
      OFFS_FLAG  + STAT_FREEZE    , \
      OFFS_EMPTY                  , \
      OFFS_HORIZ + STAT_SCRNUM    , \
      OFFS_EMPTY                  , \
      OFFS_FLAG  + STAT_CSRVIS    , \
      OFFS_FLAG  + STAT_ATTRVIS   , \
      OFFS_FLAG  + STAT_CAPBLINK  , \
      OFFS_FLAG  + STAT_SIXDOTS   , \
      OFFS_FLAG  + STAT_SKPIDLNS  , \
      OFFS_FLAG  + STAT_TUNES     , \
      OFFS_EMPTY                  , \
      OFFS_FLAG  + STAT_INPUT     , \
      OFFS_FLAG  + STAT_AUTOSPEAK , \
      OFFS_FLAG  + STAT_AUTOREPEAT, \
      OFFS_EMPTY

/* commands for 22 status keys */
#define CMDS_STAT_22 \
      CHGONOFF( CMD_HELP       , OFFS_STAT +  1, 0200, 0100 ), \
              { CMD_LEARN      , OFFS_STAT +  2, 0000       }, \
              { CMD_CSRJMP_VERT, OFFS_STAT +  3, 0000       }, \
              { CMD_BACK       , OFFS_STAT +  4, 0000       }, \
      CHGONOFF( CMD_INFO       , OFFS_STAT +  5, 0200, 0100 ), \
      CHGONOFF( CMD_CSRTRK     , OFFS_STAT +  6, 0200, 0100 ), \
      CHGONOFF( CMD_DISPMD     , OFFS_STAT +  7, 0200, 0100 ), \
      CHGONOFF( CMD_FREEZE     , OFFS_STAT +  8, 0200, 0100 ), \
              { CMD_PREFMENU   , OFFS_STAT +  9, 0000       }, \
              { CMD_PREFSAVE   , OFFS_STAT + 10, 0000       }, \
              { CMD_PREFLOAD   , OFFS_STAT + 11, 0000       }, \
      CHGONOFF( CMD_CSRVIS     , OFFS_STAT + 12, 0200, 0100 ), \
      CHGONOFF( CMD_ATTRVIS    , OFFS_STAT + 13, 0200, 0100 ), \
      CHGONOFF( CMD_CAPBLINK   , OFFS_STAT + 14, 0200, 0100 ), \
      CHGONOFF( CMD_SIXDOTS    , OFFS_STAT + 15, 0200, 0100 ), \
      CHGONOFF( CMD_SKPIDLNS   , OFFS_STAT + 16, 0200, 0100 ), \
      CHGONOFF( CMD_TUNES      , OFFS_STAT + 17, 0200, 0100 ), \
              { CMD_RESTARTBRL , OFFS_STAT + 18, 0000       }, \
      CHGONOFF( CMD_INPUT      , OFFS_STAT + 19, 0200, 0100 ), \
      CHGONOFF( CMD_AUTOSPEAK  , OFFS_STAT + 20, 0200, 0100 ), \
      CHGONOFF( CMD_AUTOREPEAT , OFFS_STAT + 21, 0200, 0100 ), \
              { CMD_PASTE      , OFFS_STAT + 22, 0000       }


static one_terminal pm_terminals[] =
{
  {
    0,				/* identity */
    "BrailleX Compact 486",	/* name of terminal */
    "brltty-pm-c-486.hlp",		/* filename of local helpfile */
    40, 1,			/* size of display */
    0,				/* number of status cells */
    9,				/* number of frontkeys */
    0,				/* terminal has an easy bar */
    {				/* status cells: info to show */
    },
    {				/* modifiers */
      MOD_FRONT_9
    },
    {				/* commands + keys */
      CMDS_FRONT_9
    },
  },

  {
    1,				/* identity */
    "BrailleX 2D Lite (plus)",	/* name of terminal */
    "brltty-pm-2d-l.hlp",		/* filename of local helpfile */
    40, 1,			/* size of display */
    13,				/* number of status cells */
    9,				/* number of frontkeys */
    0,				/* terminal has an easy bar */
    {				/* status cells: info to show */
      SHOW_STAT_13,
    },
    {				/* modifiers */
      MOD_FRONT_9
    },
    {				/* commands + keys */
      CMDS_FRONT_9,
      CMDS_STAT_13
    },
  },

  {
    2,				/* identity */
    "BrailleX Compact/Tiny",	/* name of terminal */
    "brltty-pm-c.hlp",		/* filename of local helpfile */
    40, 1,			/* size of display */
    0,				/* number of status cells */
    9,				/* number of frontkeys */
    0,				/* terminal has an easy bar */
    {				/* status cells: info to show */
    },
    {				/* modifiers */
      MOD_FRONT_9
    },
    {				/* commands + keys */
      CMDS_FRONT_9
    },
  },

  {
    3,				/* identity */
    "BrailleX 2D Screen Soft", /* name of terminal */
    "brltty-pm-2d-s.hlp",		/* filename of local helpfile */
    80, 1,			/* size of display */
    22,				/* number of status cells */
    13,				/* number of frontkeys */
    0,				/* terminal has an easy bar */
    {
      SHOW_STAT_22
    },
    {				/* modifiers */
      MOD_FRONT_13
    },
    {				/* commands + keys */
      CMDS_FRONT_13,
      CMDS_STAT_22
    },
  },

  {
    6,				/* identity */
    "BrailleX IB 80 CR Soft",	/* name of terminal */
    "brltty-pm-ib-80.hlp",		/* filename of local helpfile */
    80, 1,			/* size of display */
    4,				/* number of status cells */
    9,				/* number of frontkeys */
    0,				/* terminal has an easy bar */
    {
      SHOW_STAT_4
    },
    {				/* modifiers */
      MOD_FRONT_9
    },
    {				/* commands + keys */
      CMDS_FRONT_9,
      CMDS_STAT_4
    },
  },

  {
    64,				/* identity */
    "BrailleX EL 2D-40",	/* name of terminal */
    "brltty-pm-el-2d-40.hlp",		/* filename of local helpfile */
    40, 1,			/* size of display */
    13,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {
      SHOW_STAT_13
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_STAT_13,
      CMDS_EASY
    },
  },

  {
    65,				/* identity */
    "BrailleX EL 2D-66",	/* name of terminal */
    "brltty-pm-el-2d-66.hlp",		/* filename of local helpfile */
    66, 1,			/* size of display */
    13,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
      SHOW_STAT_13
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_STAT_13,
      CMDS_EASY
    },
  },

  {
    66,				/* identity */
    "BrailleX EL 80",		/* name of terminal */
    "brltty-pm-el-80.hlp",		/* filename of local helpfile */
    80, 1,			/* size of display */
    2,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
      SHOW_STAT_2
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_EASY,
      CMDS_STAT_2
    },
  },

  {
    67,				/* identity */
    "BrailleX EL 2D-80",		/* name of terminal */
    "brltty-pm-el-2d-80.hlp",		/* filename of local helpfile */
    80, 1,			/* size of display */
    20,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
      SHOW_STAT_20
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_EASY,
      CMDS_STAT_20
    },
  },

  {
    68,				/* identity */
    "BrailleX EL 40 P",		/* name of terminal */
    "brltty-pm-el-40-p.hlp",		/* filename of local helpfile */
    40, 1,			/* size of display */
    0,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_EASY
    },
  },

  {
    69,				/* identity */
    "BrailleX Elba 32",		/* name of terminal */
    "brltty-pm-elba-32.hlp",		/* filename of local helpfile */
    32, 1,			/* size of display */
    0,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_EASY
    },
  },

  {
    70,				/* identity */
    "BrailleX Elba 20",		/* name of terminal */
    "brltty-pm-elba-20.hlp",		/* filename of local helpfile */
    20, 1,			/* size of display */
    0,				/* number of status cells */
    0,				/* number of frontkeys */
    1,				/* terminal has an easy bar */
    {				/* status cells: info to show */
    },
    {				/* modifiers */
      MOD_EASY
    },
    {				/* commands + keys */
      CMDS_EASY
    },
  },
};

static const int num_terminals = sizeof(pm_terminals)/sizeof(pm_terminals[0]);

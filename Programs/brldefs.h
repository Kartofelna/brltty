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

#ifndef _BRLDEFS_H
#define _BRLDEFS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Argument for brl_readCommand() */
typedef enum {
  CMDS_SCREEN,
  CMDS_HELP,
  CMDS_STATUS,
  CMDS_PREFS,
  CMDS_MESSAGE
} DriverCommandContext;

/* The following define command codes, which are return values for
 * readbrl().  The CMD_* codes are guaranteed to be between 1 and 127
 * inclusive, with the exception of CMD_NOOP = 0.
 *
 * readbrl() should return EOF if there are no keys to return.  If,
 * however, it has masked out the key for internal use, it may return
 * CMD_NOOP - in which case it will be called again immediately, rather
 * than waiting for the next cycle.  This can save internal nesting ...
 */

/*
 * Please comment all CMD_* definitions. They are
 * used during automatic help file generation.
 */
typedef enum {
  /* special commands which must be first and remain in order */
  CMD_NOOP = 0 /* do nothing */,

  /* vertical motion */
  CMD_LNUP /* go up one line */,
  CMD_LNDN /* go down one line */,
  CMD_WINUP /* go up several lines */,
  CMD_WINDN /* go down several lines */,
  CMD_PRDIFLN /* go up to nearest line with different content */,
  CMD_NXDIFLN /* go down to nearest line with different content */,
  CMD_ATTRUP /* go up to nearest line with different highlighting */,
  CMD_ATTRDN /* go down to nearest line with different highlighting */,
  CMD_TOP /* go to top line */,
  CMD_BOT /* go to bottom line */,
  CMD_TOP_LEFT /* go to top-left corner */,
  CMD_BOT_LEFT /* go to bottom-left corner */,
  CMD_PRPGRPH /* go up to last line of previous paragraph */,
  CMD_NXPGRPH /* go down to first line of next paragraph */,
  CMD_PRPROMPT /* go up to previous command prompt */,
  CMD_NXPROMPT /* go down to next command prompt */,
  CMD_PRSEARCH /* search backward for content of cut buffer */,
  CMD_NXSEARCH /* search forward for content of cut buffer */,
  
  /* horizontal motion */
  CMD_CHRLT /* go left one character */,
  CMD_CHRRT /* go right one character */,
  CMD_HWINLT /* go left half a window */,
  CMD_HWINRT /* go right half a window */,
  CMD_FWINLT /* go left one window */,
  CMD_FWINRT /* go right one window */,
  CMD_FWINLTSKIP /* go left to nearest non-blank window */,
  CMD_FWINRTSKIP /* go right to nearest non-blank window */,
  CMD_LNBEG /* go to beginning of line */,
  CMD_LNEND /* go to end of line */,
  
  /* implicit motion */
  CMD_HOME /* go to cursor */,
  CMD_BACK /* go back (undo unexpected cursor tracking motion) */,
  
  /* feature activation and deactivation */
  CMD_FREEZE /* toggle screen mode [live/frozen] */,
  CMD_DISPMD /* toggle display mode [text/attributes] */,
  CMD_SIXDOTS /* toggle text style [8-dot/6-dot] */,
  CMD_SLIDEWIN /* toggle sliding window [off/on] */,
  CMD_SKPIDLNS /* toggle skipping of lines with identical content [off/on] */,
  CMD_SKPBLNKWINS /* toggle skipping of blank windows [off/on] */,
  CMD_CSRVIS /* toggle cursor visibility [on/off] */,
  CMD_CSRHIDE /* toggle hidden cursor [off/on] */,
  CMD_CSRTRK /* toggle cursor tracking [on/off] */,
  CMD_CSRSIZE /* toggle cursor style [underline,block] */,
  CMD_CSRBLINK /* toggle cursor blinking [off/on] */,
  CMD_ATTRVIS /* toggle attribute underlining [off/on] */,
  CMD_ATTRBLINK /* toggle attribute blinking [on/off] */,
  CMD_CAPBLINK /* toggle capital letter blinking [off/on] */,
  CMD_TUNES /* toggle alert tunes [on/off] */,
 
  /* mode selection */
  CMD_HELP /* enter/leave help display */,
  CMD_INFO /* enter/leave status display */,
  CMD_LEARN /* enter/leave command learn mode */,
  
  /* preference setting */
  CMD_PREFMENU /* enter/leave preferences menu */,
  CMD_PREFSAVE /* save current preferences */,
  CMD_PREFLOAD /* restore saved preferences */,
  
  /* menu navigation */
  CMD_MENU_FIRST_ITEM /* go to first item in menu */,
  CMD_MENU_LAST_ITEM /* go to last item in menu */,
  CMD_MENU_PREV_ITEM /* go to previous item in menu */,
  CMD_MENU_NEXT_ITEM /* go to next item in menu */,
  CMD_MENU_PREV_SETTING /* change value of current item in menu to previous choice */,
  CMD_MENU_NEXT_SETTING /* change value of current item in menu to next choice */,
 
  /* speech */
  CMD_SAY_LINE /* speak current line */,
  CMD_SAY_ABOVE /* speak from top of screen through current line */,
  CMD_SAY_BELOW /* speak from current line through bottom of screen */,
  CMD_MUTE /* stop speaking immediately */,
  CMD_SPKHOME /* go to current (most recent) speech position */,
  
  /* virtual terminal switching */
  CMD_SWITCHVT_PREV /* switch to previous virtual terminal */,
  CMD_SWITCHVT_NEXT /* switch to next virtual terminal */,
  
  /* miscellaneous */
  CMD_CSRJMP_VERT /* vertically route cursor to top line of window */,
  CMD_PASTE /* insert cut buffer at cursor */,
  CMD_RESTARTBRL /* reinitialize braille driver */,
  CMD_RESTARTSPEECH /* reinitialize speech driver */,
  
  DriverCommandCount /* must be last */
} DriverCommand;
#define VAL_ARG_MASK 0XFF
# define VAL_BLK_MASK 0XFF00
# define VAL_FLG_MASK 0XFF0000
  
/* For specifically turning on/off toggle commands */
#define VAL_SWITCHON    0x10000
#define VAL_SWITCHOFF   0x20000
#define VAL_SWITCHMASK  (VAL_SWITCHON | VAL_SWITCHOFF)
  
/* cursor routing keys block offset values */
/*
 * Please comment all CR_* definitions. They are
 * used during automatic help file generation.
 */
#define CR_ROUTE     0X100   /* bring cursor to character */
#define CR_CUTBEGIN  0X200 /* start new cut buffer */
#define CR_CUTAPPEND 0X300 /* append to existing cut buffer */
#define CR_CUTRECT   0X400 /* rectangular cut */
#define CR_CUTLINE   0X500 /* linear cut */
#define CR_SWITCHVT  0X600 /* switch to virtual terminal */
#define CR_PRINDENT  0X700 /* go up to nearest line without greater indent */
#define CR_NXINDENT  0X800 /* go down to nearest line without greater indent */
#define CR_DESCCHAR  0X900 /* describe character */
#define CR_SETLEFT   0XA00 /* position left end of window */
#define CR_SETMARK   0XB00 /* remember current window position */
#define CR_GOTOMARK  0XC00 /* go to remembered window position */

/* For entering a special key. */
#define VAL_PASSKEY 0x2000
typedef enum {
  VPK_RETURN,
  VPK_TAB,
  VPK_BACKSPACE,
  VPK_ESCAPE,
  VPK_CURSOR_LEFT,
  VPK_CURSOR_RIGHT,
  VPK_CURSOR_UP,
  VPK_CURSOR_DOWN,
  VPK_PAGE_UP,
  VPK_PAGE_DOWN,
  VPK_HOME,
  VPK_END,
  VPK_INSERT,
  VPK_DELETE,
  VPK_FUNCTION
} Key;

#define VAL_PASSCHAR 0x2100 /* input a character by value */
#define VAL_PASSDOTS 0x2200 /* input a character as braille dots */

/* For modifying a character to be typed. */
#define VPC_CONTROL 0X010000
#define VPC_META    0X020000
#define VPC_UPPER   0X040000
#define VPC_SHIFT   0X080000

/* status cell styles */
typedef enum {
  ST_None,
  ST_AlvaStyle,
  ST_TiemanStyle,
  ST_PB80Style,
  ST_Generic,
  ST_MDVStyle,
  ST_VoyagerStyle
} StatusCellStyles;

/*
 * Please comment all STAT_* definitions. They are
 * used during automatic help file generation.
 */
typedef enum {
  FirstStatusCell = 0,
  #define FSC_GENERIC 0XFF

  /* numbers */
  STAT_BRLCOL /* screen column where left of braille window is */,
  STAT_BRLROW /* screen row where top of braille window is */,
  STAT_CSRCOL /* screen column where cursor is */,
  STAT_CSRROW /* screen row where cursor is */,
  STAT_SCRNUM /* virtual screen number */,

  /* flags */
  STAT_FREEZE /* frozen screen */,
  STAT_DISPMD /* attributes display */,
  STAT_SIXDOTS /* six-dot braille */,
  STAT_SLIDEWIN /* sliding window */,
  STAT_SKPIDLNS /* skip identical lines */,
  STAT_SKPBLNKWINS /* skip blank windows */,
  STAT_CSRVIS /* visible cursor */,
  STAT_CSRHIDE /* hidden cursor */,
  STAT_CSRTRK /* cursor tracking */,
  STAT_CSRSIZE /* block cursor */,
  STAT_CSRBLINK /* blinking cursor */,
  STAT_ATTRVIS /* visible attributes underline */,
  STAT_ATTRBLINK /* blinking attributes underline */,
  STAT_CAPBLINK /* blinking capital letters */,
  STAT_TUNES /* alert tunes */,
  STAT_HELP /* help mode */,
  STAT_INFO /* info mode */,

  StatusCellCount /* must be last */
} StatusCell;

/* The bits for each braille dot.
 * logical layout   real bits 
 *  1 4               1 2
 *  2 5         -->   3 4
 *  3 6               5 6
 *  7 8               7 8
 * Bx: x from logical layout, value from real layout  
 */
#define B1 0X01
#define B2 0X04
#define B3 0X10
#define B4 0X02
#define B5 0X08
#define B6 0X20
#define B7 0X40
#define B8 0X80

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BRLDEFS_H */

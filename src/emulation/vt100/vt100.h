/**
 * @filename vt100.h
 * @author Darryl Pogue & Terence Stenvold
 * @designer Darryl Pogue
 * @date 2010 10 18
 * @project Terminal Emulator: VT100 Plugin
 *
 * This file contains the definitions and prototypes for functions used by
 * the VT100 emulation mode plugin.
 */
#ifndef _VT100_H
#define _VT100_H

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "../../defines.h"
#include "../../emulation.h"

#define STYLE_POS(x) (INT)((x & 0xFF000000) >> 24)

typedef struct _cursor {
    DWORD x;
    DWORD y;
    DWORD style;
} Cursor;

enum {
    kKeypadNumericMode,
    kKeypadApplicationMode,
    kCursorApplicationMode
};

enum {
    kLineNormal = 0,
    kLineDoubleWidth = (1 << 0),
    kLineDoubleTop = (1 << 1),
    kLineDoubleBottom = (1 << 2)
};

enum {
    kLineStyleNormal = 0,
    kLineStyleBold = (1 << 1),
    kLineStyleBlink = (1 << 2), /* Unsupported! */
    kLineStyleUnderline = (1 << 3),
    kLineStyleReverse = (1 << 4)
};

typedef struct _colstyle ColStyle;

/** DWORD style:
 *  [8 bits] = Column Number (0 - 132)
 *  [8 bits] = Style (bit vector)
 *  [8 bits] = Foreground Colour
 *  [8 bits] = Background Colour
 */
struct _colstyle {
    DWORD style;
    ColStyle* next;
};

typedef struct _line {
    /* Refers to double height/width */
    CHAR weight;
    BOOLEAN bDirty;
    ColStyle* colstyle;
} Line;

typedef struct _vt100_data {
    HWND hwnd;
    Cursor current;
    Cursor saved;
    Cursor origin; /* Not really a cursor */
    CHAR htabs[132];
    DWORD scroll_top;
    DWORD scroll_bottom;
    BOOLEAN autowrap;
    BOOLEAN relorigin;
    CHAR appcursormode;
    BOOLEAN screen_reverse;
    Line lines[24];
    TCHAR screen[24][81];
} VT100_Data;

/**
 * Sets the style of a line on the screen.
 * @implementation vt100_parser.c
 */
void set_line_style(ColStyle* ln, DWORD x, DWORD style);

/**
 * Sets the cursor style.
 * @implementation vt100_parser.c
 */
void set_style(VT100_Data* vt, CHAR attr, CHAR fg, CHAR bg, BOOL reset);

/**
 * Handles an escape sequence beginning with ESC[ and ending with m.
 * @implementation vt100_parser.c
 */
void escape_colour(LPCTSTR rx, VT100_Data* vt);

/**
 * Handles an escape sequence beginning with ESC[.
 * @implementation vt100_parser.c
 */
void escape_bracket(LPCTSTR rx, VT100_Data* vt);

/**
 * Handles an escape sequence beginning with ESC?.
 * @implementation vt100_parser.c
 */
void escape_question(LPCTSTR rx, VT100_Data* vt);

/**
 * Handles an escape sequence beginning with ESC#.
 * @implementation vt100_parser.c
 */
void escape_hash(LPCTSTR rx, VT100_Data* vt);

/**
 * Scrolls all the lines in the scrolling region up or down by one line.
 * @implementation vt100_parser.c
 */
void scroll_screen(char up, VT100_Data* vt);


/**
 * Draws a line of text to the screen.
 * @implementation vt100_renderer.c
 */
DWORD draw_line(DWORD ln, VT100_Data* vt, HWND hwnd, HDC hdc);

#endif

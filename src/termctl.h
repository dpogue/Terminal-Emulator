/**
 * @filename termctl.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the definitions and prototypes for the terminal
 * window "control" which are available for plugins to call.
 */
#ifndef _TERMCTL_H_
#define _TERMCTL_H_

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

typedef struct _ctl_cursor {
    POINT pPosition;
    DWORD dwStyle;
} CtlCursor;

/** 
 * Gets the terminal width in number of characters.
 * @implementation termctl.c
 */
DWORD GetTerminalWidth(HWND hwnd);

/** 
 * Gets the terminal height in number of characters.
 * @implementation termctl.c
 */
DWORD GetTerminalHeight(HWND hwnd);

POINT GetTerminalMinSize(HWND hwnd);

VOID SetTerminalMinSize(HWND hwnd, POINT size);

POINT GetTerminalMaxSize(HWND hwnd);

VOID SetTerminalMaxSize(HWND hwnd, POINT size);

/**
 * Gets whether the terminal has a scrollbar and is able to scroll up through
 * previous lines of text.
 * @implementation termctl.c
 */
BOOLEAN GetTerminalScroll(HWND hwnd);

/**
 * Gets whether the terminal will wrap text when it reaches the end of a line.
 * @implementation termctl.c
 */
BOOLEAN GetTerminalWrap(HWND hwnd);

/**
 * Gets whether the terminal will echo back characters as they are typed.
 * @implementation termctl.c
 */
BOOLEAN GetTerminalEcho(HWND hwnd);

VOID SetTerminalProperties(HWND hwnd, BOOLEAN scroll, BOOLEAN wrap, BOOLEAN echo);

CtlCursor GetTerminalCursor(HWND hwnd);

VOID SetTerminalCursorPos(HWND hwnd, DWORD x, DWORD y);

VOID SetTerminalCursorStyle(HWND hwnd, DWORD dwStyle);

/**
 * Adds text at the current position in the terminal.
 * @implementation termctl.c
 */
VOID AddTerminalText(HWND hwnd, TCHAR* lpszText);

/**
 * Adds a line of text to the terminal.
 * @implementation termctl.c
 */
VOID AddTerminalLine(HWND hwnd, TCHAR* lpszText);

#endif

/**
 * @filename termctl_private.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the definitions and prototypes for the terminal
 * window "control" which are not made available to emulation plugins.
 */
#ifndef _TERMCTL_PRIVATE_H_
#define _TERMCTL_PRIVATE_H_

#include "defines.h"
#include "termctl.h"

typedef struct _ctl_text_line {
    TCHAR* lpszText;
    DWORD* lpdwStyle;
} CtlTextLine;

typedef struct _ctl_data {
    BOOLEAN bConnected;
    BOOLEAN bScroll;
    BOOLEAN bWrap;
    BOOLEAN bEchoInput;

    POINT pMinSize;
    POINT pMaxSize;

    CtlCursor cursor;
    CtlTextLine** lpLines;
    DWORD dwNumLines;

    LOGFONT font;
    TEXTMETRIC tm;

    Emulator* lpEmulator;
    HANDLE hCommDev;
    HANDLE hPollThread;
} CtlData;

/**
 * Initialise the terminal data structure.
 * @implementation termctl.c
 */
VOID InitTerminalCtl(HWND hwnd);

/**
 * Initialise the terminal lines.
 * @implementation termctl.c
 */
VOID InitTerminalLines(CtlData* data);

/**
 * Sets the terminal font.
 * @implementation termctl.c
 */
VOID SetTerminalFont(HWND hwnd, LOGFONT font);

/**
 * Sets the emulation mode for the terminal.
 * @implementation termctl.c
 */
VOID SetTerminalEmulator(HWND hwnd, Emulator* emulator);

/**
 * Create a new line on the terminal.
 * @implementation termctl.c
 */
VOID MakeTerminalLine(CtlData* data, CtlTextLine** line);

/**
 * Paint the entire terminal window.
 * @implementation termctl.c
 */
VOID PaintTerminal(HWND hwnd, HDC hdc);

/**
 * Paint a line of the terminal.
 * @implementation termctl.c
 */
VOID PaintTerminalLine(HWND hwnd, HDC hdc, DWORD line);

#endif

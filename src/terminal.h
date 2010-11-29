/**
 * @filename terminal.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the definitions and prototypes for the terminal
 * window.
 */
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>
#include "defines.h"
#include "serial.h"
#include "emulation.h"
#include "emulation_none.h"
#include "pluginloader.h"
#include "termctl_private.h"
#include "connectdlg.h"
#include "prefsdlg.h"

/* Menu Item constants */
#define ID_EXIT 100
#define ID_DISCONNECT 101
#define ID_CONNECT 102
#define ID_PREFERENCES 103
#define ID_ABOUT 105

#define ID_EMU_START 150
#define ID_EMU_ONE 151
#define ID_EMU_TWO 152

typedef struct _terminal_modes {
    DWORD dwCount;
    Emulator** lpEmulators;
    HWND hPrefs;
} TerminalModes;

/**
 * Reports a system error to the user in a MessageBox.
 * @implementation terminal.c
 */
void ReportError(DWORD dwError);

/**
 * Prompts for connection parameters and establishes a connection.
 * @implementation terminal.c
 */
VOID Connect(HWND hwnd);

/**
 * Safely closes an open connection.
 * @implementation terminal.c
 */
VOID Disconnect(HWND hwnd);

/**
 * Registers the window class with the system.
 * @implementation terminal_win.c
 */
DWORD RegisterWindowClass(HINSTANCE hInstance);

/**
 * Creates the terminal window and initialises the instance data.
 * @implementation terminal_win.c
 */
HWND CreateTerminal(HINSTANCE hInstance);

/**
 * Pull messages off of the queue and send them for parsing.
 * @implementation terminal_win.c
 */
VOID MessageLoop(HWND hwnd);

/**
 * Message handling for Windows messages.
 * @implementation terminal_win.c
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * Message handler for the WM_PAINT message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnPaint(HWND hwnd);

/**
 * Message handler for the WM_GETMINMAXINFO message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpInfo);

/**
 * Message handler for the WM_SIZE message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnSize(HWND hwnd, UINT state, INT cx, INT cy);

/**
 * Message handler for the WM_COMMAND message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnCommand(HWND hwnd, INT id, HWND ctl, UINT notify);

/**
 * Message handler for the WM_HSCROLL message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, INT pos);

/**
 * Message handler for the WM_VSCROLL message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, INT pos);

/**
 * Message handler for the WM_KEYUP message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnKey(HWND hwnd, UINT vk, BOOL fDown, INT repeat, UINT flags);

/**
 * Message handler for the WM_CHAR message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnChar(HWND hwnd, TCHAR c);

/**
 * Message handler for the WM_DESTROY message.
 * @implementation terminal_win.c
 */
VOID Terminal_OnDestroy(HWND hwnd);

#endif

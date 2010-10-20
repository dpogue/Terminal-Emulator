/**
 * @filename terminal.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 09 24
 * @project Terminal Emulator (COMP3980 Asn1)
 *
 * This file contains the definitions and prototypes for the terminal
 * window.
 */
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <Windows.h>
#include <tchar.h>
#include "defines.h"
#include "serial.h"
#include "emulation.h"

/* MENU ITEM ID DEFINES */
#define ID_EXIT 100
#define ID_DISCONNECT 101
#define ID_CONNECT 102
#define ID_COM_START 110

/* ENUMERATION DECLARATIONS */
enum modes {
    kModeCommand = 0,
    kModeConnect = 1,
};

/* UGLY EXTERNS */
/* In an ideal world, emulation modes would be dynamically loaded as plugins
   However, this is not the best time to deal with Win32 DLL loading.  */
extern Emulator* vt100_init(HWND);

/**
 * The TermInfo structure contains information regarding the current state of
 * the terminal emulator application.

 * @member DWORD dwMode     The current application mode (command or connect)
 * @member HWND hwnd        The handle to the application window
 * @member HANDLE hCommDev  The handle to the serial port device
 * @member HANDLE hReadLoop The handle to the read thread
 * @member TCHAR screen[][] The screen buffer (25 lines, 80 chars per line)
 */
typedef struct _TermInfo {
    DWORD dwMode;
    HWND hwnd;
    HANDLE hCommDev;
    HANDLE hReadLoop;
    /*TCHAR screen[24][81];*/
    Emulator* hEmulator;
} TermInfo;

/* FUNCTION PROTOTYPES */
/**
 * Message handling for Windows messages.
 * @implementation terminal_win.c
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * Reports a system error to the user in a MessageBox.
 * @implementation terminal.c
 */
void ReportError(DWORD dwError);

/**
 * Enters command mode, closing any open ports and enabling the connect menu.
 * @implementation terminal.c
 */
void CommandMode(HWND hwnd);

/**
 * Enters connect mode, connecting to the specified port number.
 * @implementation terminal.c
 */
void ConnectMode(HWND hwnd, DWORD port);

#endif

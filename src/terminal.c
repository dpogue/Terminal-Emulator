/**
 * @filename terminal.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 09 24
 * @project Terminal Emulator (COMP3980 Asn1)
 *
 * This file contains the implementations of all general (mode, error handling,
 * and threading) functions for the terminal emulator.
 */
#include "terminal.h"


/**
 * Reports a system error to the user in a MessageBox.
 *
 * @param DWORD dwError     The system error number, often retrieved with
 *                          GetLastError()
 * @returns none
 *
 * Code largely duplicated from
 * @reference http://msdn.microsoft.com/en-us/library/ms679351(VS.85).aspx
 */
void ReportError(DWORD dwError) {
    LPVOID lpMsgBuf = NULL;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, APPNAME, MB_ICONERROR);

    LocalFree(lpMsgBuf);
}

/**
 * Enters command mode, closing any open ports and enabling the connect menu.
 *
 * @param HWND hwnd   The handle to the application window.
 * @return none
 */
void CommandMode(HWND hwnd) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    HMENU menubar = GetMenu(hwnd);
    HMENU connectmenu = CreateMenu();
    DWORD ports[10] = {0, 1, 2, 3, 4};
    DWORD max = 5;
    DWORD i = 0;

    /* If a port is already open, we should close it */
    if (ti->dwMode == kModeConnect) {
        ti->dwMode = kModeCommand;
        if (ClosePort(&ti->hCommDev) != 0) {
            DWORD dwError = GetLastError();
            ReportError(dwError);
        }
    }

    EnableMenuItem(menubar, ID_DISCONNECT, MF_GRAYED);

    /* In an ideal world, there would be an easy way to get a list of all
       available COM ports */
    for ( i = 0; i < max; i++) {
        TCHAR name[32];
        MENUITEMINFO mii;

        _stprintf_s(name, 32, TEXT("Communication Port COM&%d"), ports[i]);

        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE | MIIM_STATE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.wID = ID_COM_START + ports[i];
        mii.dwTypeData = name;

        InsertMenuItem(connectmenu, i, TRUE, &mii);
        EnableMenuItem(menubar, ID_COM_START + ports[i], MF_ENABLED);
    }

    ModifyMenu(menubar, ID_CONNECT, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)connectmenu, TEXT("&Connect"));
    EnableMenuItem(menubar, ID_CONNECT, MF_GRAYED);

    ti->dwMode = kModeCommand;
}

/**
 * The thread procedure for reading characters from the serial port in a loop
 *
 * @param LPVOID lpParameter    Pointer to a structure of data
 * @returns 0 if the thread exited successfully.
 */
static DWORD WINAPI ReadLoop(LPVOID lpParameter) {
    TermInfo* ti = (TermInfo*)lpParameter;

    while (ti->dwMode == kModeConnect) {
        if (ReadData(&ti->hCommDev, ti->hwnd) != 0) {
            DWORD dwError = GetLastError();
            ReportError(dwError);
        }
    }

    return 0;
}

/**
 * Enters connect mode, connecting to the specified port number.
 *
 * @param HWND hwnd     The handle to the application window
 * @param DWORD port    The number of the COM port to open
 * @returns none
 */
void ConnectMode(HWND hwnd, DWORD port) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    HMENU menubar = GetMenu(hwnd);
    TCHAR comport[8];
    DWORD i = 0;

    _stprintf_s(comport, 8, TEXT("COM%d"), port);

    if (OpenPort(comport, &ti->hCommDev, ti->hwnd) != 0) {
        DWORD dwError = GetLastError();
        ReportError(dwError);
        CommandMode(hwnd);
        return;
    }

    EnableMenuItem(menubar, ID_DISCONNECT, MF_ENABLED);
    EnableMenuItem(menubar, ID_CONNECT, MF_GRAYED);
    for (i = 0; i < 256; i++) {
        EnableMenuItem(menubar, ID_COM_START + i, MF_GRAYED);
    }
    ti->dwMode = kModeConnect;

    InvalidateRect(hwnd, NULL, TRUE);

    ti->hReadLoop = CreateThread(NULL, 0, &ReadLoop, (LPVOID)ti, 0, 0);

    if (EMULATOR_HAS_FUNC(ti->hEmulator, on_connect)) {
        ti->hEmulator->on_connect((LPVOID)ti->hEmulator->emulator_data);
    }
}

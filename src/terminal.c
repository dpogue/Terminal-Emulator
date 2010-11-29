/**
 * @filename terminal.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
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
 * The thread procedure for reading characters from the serial port in a loop
 *
 * @param LPVOID lpParameter    Pointer to a structure of data
 * @returns 0 if the thread exited successfully.
 */
static DWORD WINAPI ReadLoop(LPVOID lpParameter) {
    HWND hwnd = (HWND)lpParameter;
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    while (data->bConnected) {
        if (ReadData(&data->hCommDev, hwnd) != 0) {
            DWORD dwError = GetLastError();
            ReportError(dwError);
        }
    }

    CloseHandle(data->hPollThread);
    data->hPollThread = NULL;
    return 0;
}

/**
 * Prompts for connection parameters and establishes a connection.
 *
 * @param HWND hwnd     The handle to the application window
 * @return none
 */
VOID Connect(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    HMENU menubar = GetMenu(hwnd);
    /* Show the connect dialog and retrieve the user selections */
    ConnectParams* params = ShowConnectDlg(hwnd);

    if (params == NULL)
        return;

    /* Try to open the COM port */
    if (OpenPort(params->port, &data->hCommDev, hwnd, params->config) != 0) {
        DWORD dwError = GetLastError();
        ReportError(dwError);
        return;
    }

    data->bConnected = TRUE;
    data->hPollThread = CreateThread(NULL, 0, ReadLoop, (LPVOID)hwnd, 0, NULL);

    /* Enable the Disconnect & disable the Connect menu items */
    EnableMenuItem(menubar, ID_DISCONNECT, MF_ENABLED);
    EnableMenuItem(menubar, ID_CONNECT, MF_GRAYED);

    /* Repaint the application window */
    InvalidateRect(hwnd, NULL, TRUE);

    if (EMULATOR_HAS_FUNC(data->lpEmulator, on_connect)) {
        data->lpEmulator->on_connect(data->lpEmulator->emulator_data);
    }
}

/**
 * Safely closes an open connection.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return none
 */
VOID Disconnect(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    HMENU menubar = GetMenu(hwnd);

    data->bConnected = FALSE;

    /* Enable the Connect & disable the Disconnect menu items */
    EnableMenuItem(menubar, ID_CONNECT, MF_ENABLED);
    EnableMenuItem(menubar, ID_DISCONNECT, MF_GRAYED);

    /* Repaint the application window */
    InvalidateRect(hwnd, NULL, TRUE);
}

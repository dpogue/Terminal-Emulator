#ifndef _CONNECTDLG_H_
#define _CONNECTDLG_H_

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>

/* Dialog constants */
#define CONNECT_DLG 200
#define CONNECT_TYPE 201
#define CONNECT_PORT 202
#define CONNECT_CONF 203
#define CONNECT_BTN 204
#define CONNECT_QUIT 205

typedef struct _connect_params {
    TCHAR* port;
    BOOL config;
} ConnectParams;

/**
 * Displays the connection settings window as a modal dialog.
 * @implementation connectdlg.c
 */
ConnectParams* ShowConnectDlg(HWND parent);

/**
 * Message handling for Windows messages.
 * @implementation connectdlg.c
 */
LRESULT CALLBACK ConnectDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * Message handler for the WM_INITDIALOG message.
 * @implementation connectdlg.c
 */
VOID Connect_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);

/**
 * Message handler for the WM_COMMAND message.
 * @implementation connectdlg.c
 */
VOID Connect_OnCommand(HWND hDlg, INT id, HWND ctl, UINT notify);

#endif

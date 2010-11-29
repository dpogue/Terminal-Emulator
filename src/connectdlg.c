#include "connectdlg.h"

/**
 * Displays the connection settings window as a modal dialog.
 *
 * @param HWND parent   The handle to the dialog parent.
 * @return The connection parameters, or NULL.
 */
ConnectParams* ShowConnectDlg(HWND parent) {
    ConnectParams* params = (ConnectParams*)malloc(sizeof(ConnectParams));

    INT_PTR result = DialogBoxParam(GetWindowInstance(parent), MAKEINTRESOURCE(CONNECT_DLG),
                                    parent, (DLGPROC)ConnectDlgProc, (LPARAM)params);
    
    if (result == CONNECT_BTN) {
        return params;
    } else {
        return NULL;
    }
}

/**
 * Message handling for Windows messages.
 *
 * @param HWND hwnd     The handle to the window which received the event
 * @param UINT msg      The numeric value of the Windows message
 * @param WPARAM wParam Additional message data depending on the message type
 * @param LPARAM lParam Additional message data depending on the message type
 * @return zero if the message was handled, non-zero otherwise
 */
LRESULT CALLBACK ConnectDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        Connect_OnInitDialog(hDlg, (HWND)wParam, lParam);
        return TRUE;
    case WM_COMMAND:
        Connect_OnCommand(hDlg, (INT)LOWORD(wParam), (HWND)lParam, (INT)HIWORD(lParam));
        return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, CONNECT_QUIT);
        return TRUE;
    default:
        return FALSE;
    }
}

/**
 * Message handler for the WM_INITDIALOG message.
 *
 * @param HWND hDlg      The handle to the dialog.
 * @param HWND hwndFocus Unused.
 * @param LPARAM lParam  Additional initialisation data.
 * @return none
 */
VOID Connect_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam) {
    HWND types = GetDlgItem(hDlg, CONNECT_TYPE);
    HWND ports = GetDlgItem(hDlg, CONNECT_PORT);
    DWORD i = 0;
    ConnectParams* params = (ConnectParams*)lParam;

    SetWindowLongPtr(hDlg, GWL_USERDATA, (LONG)params);

    /* Populate the connection type combo list */
    ComboBox_AddString(types, TEXT("COM"));
    ComboBox_SelectString(types, 0, TEXT("COM"));

    /* Populate the connection port combo list */
    for (i = 0; i < 4; i++) {
        TCHAR port[8];

        StringCchPrintf(port, 8, TEXT("COM%d"), (i + 1));

        ComboBox_AddString(ports, port);
    }
    ComboBox_SelectString(ports, 0, TEXT("COM1"));
}

/**
 * Message handler for the WM_COMMAND message.
 *
 * @param HWND hDlg     The handle to the dialog.
 * @param INT id        The ID of the command item.
 * @param HWND ctl      The handle to the control.
 * @param UINT notify   Unused.
 * @return none
 */
VOID Connect_OnCommand(HWND hDlg, INT id, HWND ctl, UINT notify) {
    switch (id) {
    case CONNECT_BTN:
        {
            ConnectParams* params = (ConnectParams*)GetWindowLongPtr(hDlg, GWL_USERDATA);
            HWND chkConfig = GetDlgItem(hDlg, CONNECT_CONF);
            HWND port = GetDlgItem(hDlg, CONNECT_PORT);
            INT idx = ComboBox_GetCurSel(port);

            /* Store the dialog data so that it can be returned */
            params->config = (Button_GetCheck(chkConfig) == BST_CHECKED);
            params->port = (TCHAR*)malloc(ComboBox_GetLBTextLen(port, idx) * sizeof(TCHAR));
            ComboBox_GetLBText(port, idx, params->port);

            EndDialog(hDlg, id);
        }
        break;
    case CONNECT_QUIT:
        EndDialog(hDlg, id);
        break;
    }
}

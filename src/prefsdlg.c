#include "prefsdlg.h"
#include "terminal.h"

/**
 * Creates the preferences modeless dialog window.
 *
 * @param HWND parent   The parent of the dialog.
 * @return The handle to the dialog window.
 */
HWND CreatePreferencesDlg(HWND parent) {
    HWND dlg = CreateDialog(GetWindowInstance(parent), MAKEINTRESOURCE(PREFERENCES_DLG), parent, (DLGPROC)PreferencesDlgProc);

    Button_SetCheck(GetDlgItem(dlg, PREFERENCES_NONE), BST_CHECKED);

    return dlg;
}

/**
 * Shows the preferences dialog window.
 *
 * @param HWND parent   The parent of the dialog.
 * @return none
 */
VOID ShowPreferencesDlg(HWND parent) {
    TerminalModes* modes = (TerminalModes*)GetClassLongPtr(parent, 0);

    ShowWindow(modes->hPrefs, SW_SHOW);
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
LRESULT CALLBACK PreferencesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case PREFERENCES_SAVE:
            ShowWindow(hDlg, SW_HIDE);
            return TRUE;
        case PREFERENCES_NONE:
        case PREFERENCES_ONE:
        case PREFERENCES_TWO:
            SendMessage(GetParent(hDlg), WM_COMMAND, MAKEWPARAM(LOWORD(wParam) - PREFERENCES_NONE + ID_EMU_START, HIWORD(wParam)), lParam);
            return TRUE;
        }
        return FALSE;
    case WM_CLOSE:
        ShowWindow(hDlg, SW_HIDE);
        return TRUE;
    default:
        return FALSE;
    }
}

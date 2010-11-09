#include "rfid.h"
#include "../../terminal.h"

BOOL CALLBACK rfid_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
        LPARAM lParam) {

    switch(msg){
        case WM_INITDIALOG:
            {
                SetDlgItemText(hwnd, RFID_CONNSTATUS, TEXT("Disconnected"));
            }
            break;
		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) {
				case RFID_BUTTON:
					{
						HWND parent = GetParent(hwnd);

						if (IsWindowVisible(parent)) {
							ShowWindow(parent, SW_HIDE);
							SetDlgItemText(hwnd, RFID_BUTTON, TEXT("Show Terminal"));
						} else {
							ShowWindow(parent, SW_SHOW);
							SetDlgItemText(hwnd, RFID_BUTTON, TEXT("Hide Terminal"));
						}
					}
					return TRUE;
				}
			}
			break;
        case WM_CLOSE:
            {
                HWND parent = GetParent(hwnd);
                SendMessage(parent, WM_COMMAND, ID_DISCONNECT, 0);

                ShowWindow(parent, SW_SHOW);
                ShowWindow(hwnd, SW_HIDE);
            }
            return TRUE;
    }

    return FALSE;
}

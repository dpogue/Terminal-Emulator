/**
 * @filename rfid_dlg.c
 * @author Darryl Pogue & Joel Stewart
 * @designer Darryl Pogue & Joel Stewart
 * @date 2010 11 10
 * @project Terminal Emulator: RFID Plugin
 *
 * This file contains the implementation of the message loop for the
 * RFID GUI Dialog box.
 */
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
                case RFID_LED1:
                    {
                        HWND parent = GetParent(hwnd);
                        RFID_A2D_SetDriver* msg;

                        rfid_setdriver_request(&msg, 0x2);
                        SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                    }
                    return TRUE;
                case RFID_LED2:
                    {
                        HWND parent = GetParent(hwnd);
                        RFID_A2D_SetDriver* msg;

                        rfid_setdriver_request(&msg, 0x4);
                        SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                    }
                    return TRUE;
                case RFID_BUZZER:
                    {
                        HWND parent = GetParent(hwnd);
                        RFID_A2D_SetDriver* msg;

                        rfid_setdriver_request(&msg, 0x1);
                        SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                    }
                    return TRUE;
                case RFID_ISO_14443A:
                    {
                        HWND parent = GetParent(hwnd);

                        if (IsDlgButtonChecked(hwnd, LOWORD(wParam)) == BST_CHECKED) {
                            RFID_A2D_TransOn* msg;
                            rfid_transon_request(&msg, 0x2);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        } else {
                            RFID_A2D_TransOff* msg;
                            rfid_transoff_request(&msg, 0x2);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        }
                    }
                    return TRUE;
                case RFID_ISO_14443B:
                    {
                        HWND parent = GetParent(hwnd);

                        if (IsDlgButtonChecked(hwnd, LOWORD(wParam)) == BST_CHECKED) {
                            RFID_A2D_TransOn* msg;
                            rfid_transon_request(&msg, 0x3);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        } else {
                            RFID_A2D_TransOff* msg;
                            rfid_transoff_request(&msg, 0x3);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        }
                    }
                    return TRUE;
                case RFID_ISO_15693:
                    {
                        HWND parent = GetParent(hwnd);

                        if (IsDlgButtonChecked(hwnd, LOWORD(wParam)) == BST_CHECKED) {
                            RFID_A2D_TransOn* msg;
                            rfid_transon_request(&msg, 0x4);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        } else {
                            RFID_A2D_TransOff* msg;
                            rfid_transoff_request(&msg, 0x4);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        }
                    }
                    return TRUE;
                case RFID_TAG_IT_HF:
                    {
                        HWND parent = GetParent(hwnd);

                        if (IsDlgButtonChecked(hwnd, LOWORD(wParam)) == BST_CHECKED) {
                            RFID_A2D_TransOn* msg;
                            rfid_transon_request(&msg, 0x5);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        } else {
                            RFID_A2D_TransOff* msg;
                            rfid_transoff_request(&msg, 0x5);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        }
                    }
                    return TRUE;
                case RFID_LF_DST:
                    {
                        HWND parent = GetParent(hwnd);

                        if (IsDlgButtonChecked(hwnd, LOWORD(wParam)) == BST_CHECKED) {
                            RFID_A2D_TransOn* msg;
                            rfid_transon_request(&msg, 0x6);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
                        } else {
                            RFID_A2D_TransOff* msg;
                            rfid_transoff_request(&msg, 0x6);
                            SendMessage(parent, TWM_TXDATA, (WPARAM)msg, msg->header.length);
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

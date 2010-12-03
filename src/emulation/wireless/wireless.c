/**
 * @filename wireless.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 21
 * @project Terminal Emulator
 *
 * This file contains the implementation of a wireless protocol design.
 */
#include <time.h>
#include <WindowsX.h>
#include "wireless.h"
#include "resource.h"
#include "crc.h"

void SendByte(HWND hwnd, BYTE value) {
    BYTE* b = (BYTE*)malloc(sizeof(BYTE));
    b[0] = value;
    SendMessage(hwnd, TWM_TXDATA, (WPARAM)b, 1);
}

/**
 * Provides a name for the emulation mode.
 *
 * @returns A user-friendly name for this emulation.
 */
LPCTSTR wireless_emulation_name(void) {
    return TEXT("Wireless");
}

/**
 * Parses received data and handles any escape sequences, control characters
 * or terminal commands.
 *
 * @param LPVOID data   The emulation mode data
 * @param LPCTSTR rc    The received data as a string.
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD wireless_receive(LPVOID data, BYTE* rx, DWORD len) {
    WirelessData* dat = (WirelessData*)data;

    if (rx[0] == SOF && !dat->midFrame) {
        dat->read.frame = (WirelessFrame*)malloc(sizeof(WirelessFrame));
        memset(dat->read.frame, 0, CRC_SIZE + 1);
        dat->readPos = 0;
    }

    if (rx[0] == SOF || dat->midFrame) {
        DWORD i;
        for (i = 0; i < len && dat->readPos != CRC_SIZE + 1; i++) {
            ((BYTE*)dat->read.frame)[dat->readPos++] = rx[i];
        }

        dat->midFrame = (dat->readPos != CRC_SIZE + 1);
        if (dat->midFrame)
            return 0;

        rx[0] = 0;

        if (!verify_frame(dat->read.frame)) {
            TCHAR err_count[8];
            TCHAR ber_value[16];
            StringCchPrintf(err_count, 8, TEXT("%d"), ++dat->nErrors);
            SetDlgItemText(dat->hDlg, NUMBER_OF_NAKS, err_count);

            if (dat->nPackets == 0) {
                StringCchPrintf(ber_value, 8, TEXT("0.00000%"));
            } else {
                StringCchPrintf(ber_value, 8, TEXT("%f%"), ((DOUBLE)(dat->nErrors) / dat->nPackets) * 100);
            }
            SetDlgItemText(dat->hDlg, BIT_ERROR_RATE, ber_value);

            dat->readPos = 0;
            free(dat->read.frame);
            dat->read.frame = NULL;
            dat->midFrame = FALSE;
            return 0;
        }
    }

    dat->readPos = 0;
    
    switch (dat->state)
    {
        
    case kIdleState:
        if (rx[0] == ENQ) {
            if (dat->timeout == kRandDelayTimer) {
                KillTimer(dat->hwnd, dat->timeout);
                dat->timeout = 0;
            }
            dat->state = kGotENQState;
            SendByte(dat->hwnd, ACK);
            dat->state = kReadFrameState;
            dat->timeout = SetTimer(dat->hwnd, kReadFrameTimer, TO_ENQ, &ReadFrameTimeout);
        }
        break;
    case kWaitFrameACKState:
        if (rx[0] == RVI) {
            TCHAR ack_count[8];
            KillTimer(dat->hwnd, dat->timeout);
            dat->counters[dat->timeout] = 0;
            dat->timeout = 0;
            StringCchPrintf(ack_count, 8, TEXT("%d"), ++dat->nAcks);
            SetDlgItemText(dat->hDlg, NUMBER_OF_ACKS, ack_count);
            dat->state = kGotRVIState;
            SendByte(dat->hwnd, ACK);
            dat->state = kReadFrameState;
            dat->timeout = SetTimer(dat->hwnd, kReadFrameTimer, TO_READ, &ReadFrameTimeout);
            dat->send.sequence++;
            break;
        } else if (rx[0] == ACK) {
            dat->send.sequence++;
        }
        /* Fallthrough */
    case kSentENQState:
        if (rx[0] == ACK) {
            TCHAR ack_count[8];

            KillTimer(dat->hwnd, dat->timeout);
            dat->counters[dat->timeout] = 0;
            dat->timeout = 0;
            StringCchPrintf(ack_count, 8, TEXT("%d"), ++dat->nAcks);
            SetDlgItemText(dat->hDlg, NUMBER_OF_ACKS, ack_count);

            dat->state = kSendingState;
            if (dat->send.fd != NULL) {
                WirelessFrame* tosend = (WirelessFrame*)malloc(sizeof(WirelessFrame));
                TCHAR* dbgmsg = (TCHAR*)malloc(sizeof(TCHAR) * 16);
				TCHAR packets[8];

                if (dat->send.frame != NULL) {
                    free(dat->send.frame);
                }

                dat->send.frame = build_frame(dat);
                memcpy(tosend, dat->send.frame, sizeof(WirelessFrame));
				StringCchPrintf(packets, 8, TEXT("%d"), ++dat->nPackets);
				SetDlgItemText(dat->hDlg, IDC_EDIT2, packets);

                StringCchPrintf(dbgmsg, 16, TEXT("Sent frame %d.\n"), tosend->sequence);
                OutputDebugString(dbgmsg);
                free(dbgmsg);

                SendMessage(dat->hwnd, TWM_TXDATA, (WPARAM)tosend, sizeof(WirelessFrame));
                dat->state = kWaitFrameACKState;
                dat->timeout = SetTimer(dat->hwnd, kWaitFrameACKTimer, TO_FRAME, &WaitFrameACKTimeout);
            } else {
                DWORD e;
                UINT rand_timer = rand() * (RAND_MAX + 1) % TO_RAND_MAX + TO_RAND_MIN;
                SendByte(dat->hwnd, EOT);
                e = IsWindow(dat->hDlg);
                SetLastError(0);
                Button_Enable(GetDlgItem(dat->hDlg, SEND_FILE), TRUE);
                Button_Enable(GetDlgItem(dat->hDlg, SELECT_FILE), TRUE);
                e = GetLastError();
                dat->state = kIdleState;
                dat->timeout = SetTimer(dat->hwnd, kRandDelayTimer, rand_timer, &RandDelayTimeout);
                SetClassLong(dat->hDlg, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
            }
        }
        break;
    case kReadFrameState:
        if (rx[0] == EOT) {
            if (dat->read.fd != NULL) {
                fclose(dat->read.fd);
                dat->read.fd = NULL;
            }
            KillTimer(dat->hwnd, dat->timeout);
            dat->counters[dat->timeout] = 0;
            dat->timeout = 0;
            dat->state = kIdleState;
            if (dat->send.fd != NULL) {
                SendByte(dat->hwnd, ENQ);
                dat->state = kSentENQState;
                dat->timeout = SetTimer(dat->hwnd, kWaitFrameACKTimer, TO_FRAME, &WaitFrameACKTimeout);
            } else {
                UINT rand_timer = rand() * (RAND_MAX + 1) % TO_RAND_MAX + TO_RAND_MIN;
                dat->timeout = SetTimer(dat->hwnd, kRandDelayTimer, rand_timer, &RandDelayTimeout);
            }
        } else if (rx[0] == ENQ) {
            KillTimer(dat->hwnd, dat->timeout);
            dat->counters[dat->timeout] = 0;
            dat->timeout = 0;
            dat->state = kGotENQState;
            SendByte(dat->hwnd, ACK);
            dat->state = kReadFrameState;
            dat->timeout = SetTimer(dat->hwnd, kReadFrameTimer, TO_READ, &ReadFrameTimeout);
        } else if (dat->read.frame != NULL) {
            TCHAR ber_value[16];
            KillTimer(dat->hwnd, dat->timeout);
            dat->counters[dat->timeout] = 0;
            dat->timeout = 0;
            if (dat->nPackets == 0) {
                StringCchPrintf(ber_value, 8, TEXT("0.00000%"));
            } else {
                StringCchPrintf(ber_value, 8, TEXT("%f%"), ((DOUBLE)(dat->nErrors) / dat->nPackets) * 100);
            }
            SetDlgItemText(dat->hDlg, BIT_ERROR_RATE, ber_value);
            if (dat->read.fd == NULL) {
                SYSTEMTIME st;
                TCHAR* filename = (TCHAR*)malloc(sizeof(TCHAR) * 32);
                GetLocalTime(&st);

                StringCchPrintf(filename, 32, TEXT("E:\\recv_%04d%02d%02d%02d%02d%02d.txt"),
                        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

                dat->read.fd = _tfopen(filename, TEXT("ab"));
            }

            if ((dat->read.sequence & 0x1) == (dat->read.frame->sequence & 0x1)) {
                static int recv_frame = 1;
                TCHAR* dbgmsg = (TCHAR*)malloc(sizeof(TCHAR) * 16);

                StringCchPrintf(dbgmsg, 16, TEXT("Got frame %d.\n"), recv_frame++);
                OutputDebugString(dbgmsg);
                free(dbgmsg);

                fwrite(dat->read.frame->data, 1, dat->read.frame->size, dat->read.fd);
                fflush(dat->read.fd);
                dat->read.sequence ^= 0x1;
            }

            if (dat->read.frame->size != READ_SIZE) {
                if (dat->read.fd != NULL) {
                    fclose(dat->read.fd);
                    dat->read.fd = NULL;
                }
            }

            if (dat->send.fd != NULL) {
                SendByte(dat->hwnd, RVI);
                dat->state = kSentENQState;
                dat->timeout = SetTimer(dat->hwnd, kSentENQTimer, TO_ENQ, &SentENQTimeout);
            } else {
                SendByte(dat->hwnd, ACK);
                dat->state = kReadFrameState;
                dat->timeout = SetTimer(dat->hwnd, kReadFrameTimer, TO_READ, &ReadFrameTimeout);
            }
            free(dat->read.frame);
            dat->read.frame = NULL;
        }
        break;
    }
    return 0;
}

/**
 * Paint the screen according to the rules of this emulation mode.
 *
 * @param HWND hwnd     Handle to the application window.
 * @param LPVOID data   The emulation mode data
 * @param HDC hdc       The handle to the device context.
 *                      If this is NULL, GetDC will be called.
 * @param BOOLEAN force Force a repaint of the whole screen if true.
 *
 * @returns int 0 on succes, greater than 0 otherwise.
 */
DWORD wireless_paint(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force) {
    /*NoneData* dat = (NoneData*)data;
    TEXTMETRIC tm;
    BYTE y = 0;
    BOOLEAN bGotDC = FALSE;

    if (hdc == NULL) {
        hdc = GetDC(hwnd);
        bGotDC = TRUE;
    }

    SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
    GetTextMetrics(hdc, &tm);
    SetTextCharacterExtra(hdc, 1);

    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));

    for (y = 0; y < 24; y++) {
        TextOut(hdc, 0, y * (tm.tmExternalLeading + tm.tmHeight), dat->screen[y], _tcslen(dat->screen[y]));
    }

    if (bGotDC) {
        ReleaseDC(hwnd, hdc);
    }*/

    return 0;
}

/**
 * Performs any actions that are necessary immediately after connecting with
 * this emulation mode.
 *
 * @param LPVOID data   The emulation mode data
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD wireless_on_connect(LPVOID data) {
    WirelessData* dat = (WirelessData*)data;
    DWORD x = 0;
    DWORD y = 0;
    UINT rand_timer = rand() * (RAND_MAX + 1) % TO_RAND_MAX + TO_RAND_MIN;
    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(TEXT("wireless.dll"));

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            dat->screen[y][x] = (x == 80) ? '\0' : ' ';
        }
    }
    x = (DWORD)&(dat->hDlg);
    dat->screenrow = 0;

    dat->state = kIdleState;
    dat->send.fd = NULL;
    dat->send.sequence = 0;
    dat->send.frame = NULL;

    dat->read.fd = NULL;
    dat->read.sequence = 0;
    dat->read.frame = NULL;

    dat->readPos = 0;
    dat->midFrame = FALSE;

    dat->timeout = 0;

    for (x = 0; x < kMaxTimer; x++) {
        dat->counters[x] = 0;
    }

    dat->timeout = SetTimer(dat->hwnd, kRandDelayTimer, rand_timer, &RandDelayTimeout);
    dat->nAcks = 0;
    dat->nErrors = 0;
    dat->nPackets = 0;

    dat->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(StatDialog), dat->hwnd,
                                    (DLGPROC)WirelessDlgProc, (LPARAM)dat);
    ShowWindow(dat->hDlg, SW_SHOW);

    return 0;
}

/**
 * Allows the emulation plugin to override some of the default message loop
 * handling.
 *
 * @param LPVOID data   The emulation mode data
 * @param LPMSG msg     The pointer to the window message
 * @returns BOOLEAN     TRUE if the message was handled,
 *                      FALSE otherwise
 */
BOOLEAN wireless_wnd_proc(LPVOID data, LPMSG msg) {
    WirelessData* dat = (WirelessData*)data;

    if (IsWindow(dat->hDlg) && IsDialogMessage(dat->hDlg, msg)) {
        return TRUE;
    }

    return FALSE;
}

Emulator emu_wireless =
{
    3,                           /** << Emulator structure version */
    NULL,                        /** << Emulator data pointer */
    &wireless_emulation_name,    /** << Function returning emulator name */
    NULL,                        /** << Function to escape keyboard input */
    &wireless_receive,           /** << Function to handled received data */
    &wireless_paint,             /** << Function to repaint the screen */
    &wireless_on_connect,        /** << Function to call upon connection */
    NULL,                        /** << Function to call upon disconnection */
    &wireless_wnd_proc,          /** << Function to override message loop */
    NULL                         /** << Function to return menu handle */
};

Emulator* wireless_init(HWND hwnd) {
    Emulator* e = &emu_wireless;
    WirelessData* data = (WirelessData*)malloc(sizeof(WirelessData));
    data->hwnd = hwnd;

    srand((UINT)time(NULL));

    e->emulator_data = data;

    crcInit();

    return e;
}

EMULATOR_INIT_PLUGIN(wireless_init)

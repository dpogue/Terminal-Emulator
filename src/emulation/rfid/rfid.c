/**
 * @filename rfid.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 10
 * @project Terminal Emulator: RFID Plugin
 *
 * This file contains the plugin functions for the RFID emulation plugin.
 */
#include "rfid.h"
#include "../../terminal.h"

/**
 * Provides a name for the emulation mode.
 *
 * @returns A user-friendly name for this emulation.
 */
LPCTSTR rfid_emulation_name(void) {
    return TEXT("RFID");
}

/**
 * Escapes keyboard input for this emulation mode before passing it to the
 * transmission layer.
 *
 * @param LPVOID data   The emulator data field
 * @param DWORD input   The keyboard input to be escaped.
 *                      This stores the virtual key code that was pressed.
 *
 * @returns NULL if no escaping is needed, otherwise the escaped sequence.
 */
LPCSTR rfid_escape_input(LPVOID data, DWORD input) {
    return NULL;
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
DWORD rfid_receive(LPVOID data, BYTE* rx, DWORD len) {
    static BYTE* buffer = NULL;
    static BYTE bcc = 0;
    static WORD buflen = 0;
    static WORD length = 0;

    RFID_Data* dat = (RFID_Data*)data;

    if (buffer != NULL) {
        DWORD i = 0;
        for (i = 0; i < len; i++) {
            bcc ^= rx[i];
            buffer[buflen] = rx[i];
            buflen++;
        }
    } else {
        DWORD i = 0;

        if (rx[0] != 0x01) {
            /* Expected start of frame and got... something else */
            return 1;
        }

        length = (WORD)rx[1];

        buffer = (BYTE*)calloc(1, length);

        for (i = 0; i < len; i++) {
            bcc ^= rx[i];
            buffer[i] = rx[i];
            buflen++;
        }
    }

    if (buflen == length) { /* We have the entire message */
        INT i = 0;
        LPTSTR prt;
        RFID_Header head;

        /* Check the BCC bytes to make sure it's correct */
        if (buffer[buflen - 1] != bcc && buffer[buflen] != (bcc ^ 0xFF)) {
            free(buffer);
            buffer = NULL;
            buflen = 0;
            length = 0;
            bcc = 0;
            return 2;
        }

        head.soframe = (BYTE)*(buffer);
        head.length = (WORD)*(buffer + 1);
        head.deviceID = (BYTE)*(buffer + 3);
        head.command1 = (BYTE)*(buffer + 4);
        head.command2 = (BYTE)*(buffer + 5);

        switch (head.command2) {
        case 0x40:
            {
                RFID_D2A_GetVersion msg;
                RFID_A2D_FindToken* nextmsg = NULL;
                WORD numMessages = (head.length - sizeof(RFID_Header) - 2 - 2) / 3;
                WORD pos = 7;

                msg.header = head;
                msg.status = (BYTE)*(buffer + 6);

                while (numMessages-- > 0) {
                    BYTE entity = (BYTE)*(buffer + pos);
                    WORD version = (WORD)(*(buffer + (pos + 1)) << 8 | *(buffer + (pos + 2)));

                    if (entity > 0x1 && entity < 0x9) {
                        CheckDlgButton(dat->dialog, (entity + RFID_ISO_14443A - 2), BST_CHECKED);
                    }

                    prt = (LPTSTR)malloc(sizeof(TCHAR)*80);
                    prt[0] = 0;
                    StringCchPrintf(prt, 80,
                            TEXT("    version %d.%d.%d (%s Module)"),
                            (version & 0xF00) >> 8, (version & 0xF0) >> 4,
                            (version & 0xF), rfid_entity_name(entity));
                    StringCchCopy(dat->screen[dat->screenrow++], 80, prt);
                    pos += 3;
                }

                rfid_findtoken_request(&nextmsg);
                SendMessage(dat->console, TWM_TXDATA, (WPARAM)nextmsg, nextmsg->header.length);
            }
            break;
        case 0x41:
            {
                RFID_A2D_FindToken* nextmsg = NULL;
                RFID_D2A_FindToken msg;

                msg.header = head;
                msg.status = (BYTE)*(buffer + 6);
                msg.entityID = (BYTE)*(buffer + 7);

                if (msg.status == RFIDERROR_NONE) {
                    /* Parse! */
                    prt = (LPTSTR)malloc(80);
                    prt[0] = 0;
                    StringCchPrintf(prt, 80, TEXT("%s: \0"), rfid_entity_name(msg.entityID));
                    for (i = 8; i < (length - 2); i++) {
                        TCHAR tok[4];
                        StringCchPrintf(tok, 4, TEXT("%02X \0"), buffer[i]);
                        StringCchCat(prt, 80, tok);
                    }
                    SetDlgItemText(dat->dialog, RFID_TAGFIELD, prt);
                    StringCchCopy(dat->screen[dat->screenrow++], 80, prt);
                }

                rfid_findtoken_request(&nextmsg);
                SendMessage(dat->console, TWM_TXDATA, (WPARAM)nextmsg, nextmsg->header.length);
            }
            break;
        case 0x43:
        case 0x48:
        case 0x49:
            {
                RFID_A2D_FindToken* nextmsg = NULL;
                rfid_findtoken_request(&nextmsg);
                SendMessage(dat->console, TWM_TXDATA, (WPARAM)nextmsg, nextmsg->header.length);
            }
            break;
        }

        free(buffer);
        buffer = NULL;
        buflen = 0;
        length = 0;
        bcc = 0;

        if (dat->screenrow >= 24) {
            DWORD x;
            DWORD y;

            for (y = 0; y < 23; y++) {
                TCHAR* below = dat->screen[y+1];
                StringCchCopy(dat->screen[y], 80, below);
            }

            dat->screenrow--;
            for (x = 0; x < 80; x++) {
                dat->screen[dat->screenrow][x] = ' ';
            }
        }

        InvalidateRect(dat->console, NULL, TRUE);
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
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD rfid_paint(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force) {
    RFID_Data* dat = (RFID_Data*)data;
    TEXTMETRIC tm;
    BYTE y = 0;
    BOOLEAN bGotDC = FALSE;

    if (hdc == NULL) {
        hdc = GetDC(hwnd);
        bGotDC = TRUE;
    }

    SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
    GetTextMetrics(hdc, &tm);

    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));

    for (y = 0; y < 24; y++) {
        TextOut(hdc, 0, y * (tm.tmExternalLeading + tm.tmHeight), dat->screen[y], _tcslen(dat->screen[y]));
    }

    if (bGotDC) {
        ReleaseDC(hwnd, hdc);
    }

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
DWORD rfid_on_connect(LPVOID data) {
    RFID_Data* dat = (RFID_Data*)data;
    RFID_A2D_GetVersion* msg = NULL;
    RFID_A2D_SetDriver* msg_init = NULL;
    DWORD x = 0;
    DWORD y = 0;
    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(TEXT("rfid.dll"));

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            dat->screen[y][x] = (x == 80) ? '\0' : ' ';
        }
    }
    StringCchCopy(dat->screen[0], 80, TEXT("RFID Reader"));
    dat->screen[0][11] = 0;
    dat->screenrow = 1;

    dat->dialog = CreateDialog(hInst, MAKEINTRESOURCE(RFIDDIALOG),
                    dat->console, rfid_wnd_proc);
    SetWindowLongPtr(dat->dialog, GWL_USERDATA, (LONG)data);

    SetDlgItemText(dat->dialog, RFID_CONNSTATUS, TEXT("Connected"));

    ShowWindow(dat->dialog, SW_SHOW);
    ShowWindow(dat->console, SW_HIDE);

    rfid_setdriver_request(&msg_init, 0x7);
    msg_init->active = 0x2;
    SendMessage(dat->console, TWM_TXDATA, (WPARAM)msg_init, msg_init->header.length);

    rfid_getversion_request(&msg);
    SendMessage(dat->console, TWM_TXDATA, (WPARAM)msg, msg->header.length);
    return 0;
}

/**
 * Performs any actions that are necessary immediately before disconnecting
 * with this emulation mode.
 *
 * @param LPVOID data   The emulation mode data
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD rfid_on_disconnect(LPVOID data) {
    RFID_Data* dat = (RFID_Data*)data;

    DestroyWindow(dat->dialog);
    dat->dialog = NULL;

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
BOOLEAN rfid_wnd_proc_override(LPVOID data, LPMSG msg) {
    RFID_Data* dat = (RFID_Data*)data;

    if (IsWindow(dat->dialog) && IsDialogMessage(dat->dialog, msg)) {
        return TRUE;
    }

    return FALSE;
}

Emulator emu_rfid =
{
    3,                       /** << Emulator structure version */
    NULL,                    /** << Emulator data pointer */
    &rfid_emulation_name,    /** << Function returning emulator name */
    &rfid_escape_input,      /** << Function to escape keyboard input */
    &rfid_receive,           /** << Function to handled received data */
    &rfid_paint,             /** << Function to repaint the screen */
    &rfid_on_connect,        /** << Function to call upon connection */
    &rfid_on_disconnect,     /** << Function to call upon disconnection */
    &rfid_wnd_proc_override, /** << Function to override message loop */
    NULL                     /** << Function to return menu handle */
};

Emulator* rfid_init(HWND hwnd) {
    Emulator* e = &emu_rfid;
    RFID_Data* data = (RFID_Data*)malloc(sizeof(RFID_Data));

    data->console = hwnd;
    data->dialog = NULL;
    data->screenrow = 1;

    e->emulator_data = data;

    return e;
}

EMULATOR_INIT_PLUGIN(rfid_init)

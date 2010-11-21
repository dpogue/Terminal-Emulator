/**
 * @filename wireless.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 11
 * @project Terminal Emulator
 *
 * This file contains the implementation of a barebones emulation mode.
 */

#include "wireless.h"

/**
 * Provides a name for the emulation mode.
 *
 * @returns A user-friendly name for this emulation.
 */
LPCTSTR none_emulation_name(void) {
    return TEXT("Wireless");
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
LPCSTR none_escape_input(LPVOID data, DWORD input) {
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
DWORD none_receive(LPVOID data, BYTE* rx, DWORD len) {
    NoneData* dat = (NoneData*)data;
    LPCTSTR in = (LPCTSTR)rx;
    DWORD i = 0;

    for (i = 0; i < len; i++) {
        dat->screen[dat->screenrow][dat->screencol++] = in[i];

        if (dat->screencol >= 80) {
            dat->screenrow++;
            dat->screencol = 0;
        }

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
DWORD none_paint(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force) {
    NoneData* dat = (NoneData*)data;
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
DWORD none_on_connect(LPVOID data) {
    NoneData* dat = (NoneData*)data;
    DWORD x = 0;
    DWORD y = 0;

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            dat->screen[y][x] = (x == 80) ? '\0' : ' ';
        }
    }
    dat->screenrow = 0;
    dat->screencol = 0;

    return 0;
}

Emulator emu_wireless =
{
    3,                       /** << Emulator structure version */
    NULL,                    /** << Emulator data pointer */
    &none_emulation_name,    /** << Function returning emulator name */
    &none_escape_input,      /** << Function to escape keyboard input */
    &none_receive,           /** << Function to handled received data */
    &none_paint,             /** << Function to repaint the screen */
    &none_on_connect,        /** << Function to call upon connection */
    NULL,                    /** << Function to call upon disconnection */
    NULL,                    /** << Function to override message loop */
    NULL                     /** << Function to return menu handle */
};

Emulator* wireless_init(HWND hwnd) {
    Emulator* e = &emu_wireless;
    /*NoneData* data = (NoneData*)malloc(sizeof(NoneData));

    data->screenrow = 0;
    data->screencol = 0;

    e->emulator_data = data;*/

    return e;
}

EMULATOR_INIT_PLUGIN(wireless_init)

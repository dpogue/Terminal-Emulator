/**
 * @filename vt100.c
 * @author Darryl Pogue & Terence Stenvold
 * @designer Darryl Pogue
 * @date 2010 10 18
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the "plugin" function for the VT100 emulation module.
 */
#include "vt100.h"

/**
 * Provides a name for the emulation mode.
 *
 * @returns A user-friendly name for this emulation.
 */
LPCTSTR vt100_emulation_name(void) {
    return TEXT("VT100");
}

/**
 * Escapes keyboard input for this emulation mode before passing it to the
 * transmission layer.
 *
 * @param LPVOID data   The emulator data field (VT100_Data*)
 * @param DWORD input   The keyboard input to be escaped.
 *                      This stores the virtual key code that was pressed.
 *
 * @returns NULL if no escaping is needed, otherwise the escaped sequence.
 */
LPCSTR vt100_escape_input(LPVOID data, DWORD input) {
    VT100_Data* vt = (VT100_Data*)data;

    switch(input) {
    case VK_UP:
        return (vt->appcursormode & kCursorApplicationMode) ? "\033OA\0"
                : "\033[A\0";
    case VK_DOWN:
        return (vt->appcursormode & kCursorApplicationMode) ? "\033OB\0"
                : "\033[B\0";
    case VK_RIGHT:
        return (vt->appcursormode & kCursorApplicationMode) ? "\033OC\0"
                : "\033[C\0";
    case VK_LEFT:
        return (vt->appcursormode & kCursorApplicationMode) ? "\033OD\0"
                : "\033[D\0";
    default:
        return NULL;
    }
}

/**
 * Parses received data and handles any escape sequences, control characters
 * or terminal commands.
 *
 * @param LPVOID data   The emulation mode data (VT100_Data*)
 * @param LPCTSTR rc    The received data as a string.
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD vt100_receive(LPVOID data, BYTE* rx, DWORD len) {
    static TCHAR esc_buffer[16];

    VT100_Data* vtdata = (VT100_Data*)data;
    HWND hwnd = vtdata->hwnd;
    TCHAR* trx = (TCHAR*)malloc(sizeof(TCHAR)*len);
    LPTSTR str;
    DWORD i = 0;

    for (i = 0; i < len; i++) {
        trx[i] = 0;
        trx[i] = rx[i];
    }
    trx[i] = 0;

    str = (LPTSTR)trx;

    while (_tcsclen(str) > 0) {
        if (_tcsclen(esc_buffer) != 0 && str[0] > ' ') {
            if (isalpha(str[0]) ||
                    (str[0] == '7' && _tcsclen(esc_buffer) == 1) ||
                    (str[0] == '8' && _tcsclen(esc_buffer) == 1) ||
                    (str[0] == '=' && _tcsclen(esc_buffer) == 1) ||
                    (str[0] == '>' && _tcsclen(esc_buffer) == 1) ||
                    (isdigit(str[0]) && _tcsclen(esc_buffer) == 2
                    && esc_buffer[1] == '#')) {
                LPTSTR tmp = esc_buffer;
                StringCchCat(esc_buffer, 16, str[0]);
                /*_stprintf(esc_buffer, TEXT("%s%c\0"), tmp, str[0]);*/

                OutputDebugString(esc_buffer + 1);

                if (esc_buffer[0] == 0x1B) {
                    switch(esc_buffer[1]) {
                    case '[':
                        if (str[0] == 'm') {
                            escape_colour((esc_buffer + 2), vtdata);
                        } else {
                            escape_bracket((esc_buffer + 2), vtdata);
                        }
                        break;
                    case '?':
                        escape_question((esc_buffer + 2), vtdata);
                        break;
                    case '#':
                        escape_hash((esc_buffer + 2), vtdata);
                        break;
                    case '=':
                        vtdata->appcursormode |= kKeypadApplicationMode;
                        break;
                    case '>':
                        vtdata->appcursormode &= ~kKeypadApplicationMode;
                        break;
                    case 'E':
                        vtdata->current.x = 0;
                        /* Fall through */
                    case 'D':
                        {
                            for (vtdata->current.y += 1;
                                vtdata->current.y > vtdata->scroll_bottom;
                                vtdata->current.y -= 1) {
                                    scroll_screen(1, vtdata);
                            }
                        }
                        break;
                    case 'M':
                        {
                            if (vtdata->current.y > vtdata->scroll_top) {
                                vtdata->current.y -= 1;
                            } else {
                                scroll_screen(0, vtdata);
                            }
                        }
                        break;
                    case 'H':
                        vtdata->htabs[vtdata->current.x] = 1;
                        break;
                    case '7':
                        vtdata->saved = vtdata->current;
                        break;
                    case '8':
                        vtdata->current = vtdata->saved;
                        break;
                    default:
                        OutputDebugString(TEXT("\tUnhandled!"));
                        break;
                    }
                }

                OutputDebugString(TEXT("\n"));
                esc_buffer[0] = 0;
            } else {
                DWORD len = _tcsclen(esc_buffer);
                if (len == 16) {
                    esc_buffer[0] = 0;
                } else if (esc_buffer[len-1] == '0' && str[0] == '0') {
                    /* Ignore crazy amounts of leading zeros! */
                } else {
                    LPTSTR tmp = esc_buffer;
                    StringCchCat(esc_buffer, 16, str[0]);
                    /*_stprintf(esc_buffer, TEXT("%s%c\0"), tmp, str[0]);*/
                }
            }
        } else {
            TCHAR t = str[0];

            if (t == 0x1B) {
                StringCchPrintf(esc_buffer, 16, TEXT("%c"), str[0]);
                /*_stprintf(esc_buffer, TEXT("%c\0"), t);*/
            } else if (t == '\a') {
                MessageBeep(MB_OK);
            } else if (t == '\b') {
                vtdata->current.x -= (vtdata->current.x == 0 ? 0 : 1);
            } else if (t == '\t') {
                do {
                    vtdata->current.x += (vtdata->current.x >= 80 ? 0 : 1);
                    set_line_style(vtdata->lines[vtdata->current.y].colstyle,
                        vtdata->current.x, vtdata->current.style);
                } while(vtdata->htabs[vtdata->current.x] != 1
                        && vtdata->current.x < 80);
            } else if (t == '\n' || t == 0xB || t == 0xC) {
                for (vtdata->current.y += 1;
                        vtdata->current.y > vtdata->scroll_bottom;
                        vtdata->current.y -= 1) {
                    scroll_screen(1, vtdata);
                }
                set_line_style(vtdata->lines[vtdata->current.y].colstyle,
                    vtdata->current.x, vtdata->current.style);
            } else if (t == '\r') {
                vtdata->current.x = 0;
            } else if (t == 0xE || t == 0xF) {
                /* Ignore the shift-in/out characters */
            } else {
                if (vtdata->current.x >= 80) {
                    if (vtdata->autowrap) {
                        vtdata->current.x = 0;
                        vtdata->current.y += 1;
                    } else {
                        vtdata->current.x = 79;
                    }
                }

                vtdata->screen[vtdata->current.y][vtdata->current.x] = t;

                vtdata->lines[vtdata->current.y].bDirty = TRUE;

                vtdata->current.x += 1;

                set_line_style(vtdata->lines[vtdata->current.y].colstyle,
                    vtdata->current.x, vtdata->current.style);

            }
        }

        str++;
    }

    return 0;
}

/**
 * Paint the screen according to the rules of this emulation mode.
 *
 * @param HWND hwnd     Handle to the application window.
 * @param LPVOID data   The emulation mode data (VT100_Data*)
 * @param HDC hdc       The handle to the device context.
 *                      If this is NULL, GetDC will be called.
 * @param BOOLEAN force Force a repaint of the whole screen if true.
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD vt100_paint(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force) {
    DWORD y;
    DWORD ret = 0;
    VT100_Data* vt = (VT100_Data*)data;

    for (y = 0; y < 24 && ret == 0; y++) {
        if (!vt->lines[y].bDirty && !force)
            continue;

        ret = draw_line(y, vt, hwnd, hdc);
    }

    return ret;
}

/**
 * Performs any actions that are necessary immediately after connecting with
 * this emulation mode. None for VT100.
 *
 * @param LPVOID data   The emulation mode data (VT100_Data*)
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD vt100_on_connect(LPVOID data) { 
    return 0;
}

Emulator emu_vt100 =
{
    2,                      /** << Emulator structure version */
    NULL,                   /** << Emulator data pointer */
    &vt100_emulation_name,  /** << Function returning emulator name */
    &vt100_escape_input,    /** << Function to escape keyboard input */
    &vt100_receive,         /** << Function to handled received data */
    &vt100_paint,           /** << Function to repaint the screen */
    &vt100_on_connect       /** << Function to call upon connection */
};

/**
 * Initialisation function for the VT100 emulation mode.
 *
 * @param HWND hwnd    The handle to the application window.
 *
 * @returns A pointer to the initialised emulation mode plugin struct.
 */
Emulator* vt100_init(HWND hwnd) {
    Emulator* e = &emu_vt100;
    VT100_Data* vt = (VT100_Data*)malloc(sizeof(VT100_Data));
    DWORD x;
    DWORD y;

    e->emulator_data = vt;

    vt->hwnd = hwnd;

    for (x = 0; x < 80; x++) {
        vt->htabs[x] = 0;
    }
    vt->htabs[79] = 1;

    vt->current.x = 0;
    vt->current.y = 0;
    vt->current.style = 0x00000700;
    vt->saved.x = 0;
    vt->saved.y = 0;
    vt->saved.style = 0x00000700;

    vt->origin.x = 0;
    vt->origin.y = 0;
    vt->origin.style = 0;

    vt->scroll_bottom = 23;
    vt->scroll_top = 0;

    vt->autowrap = TRUE;
    vt->relorigin = FALSE;
    vt->appcursormode = kKeypadNumericMode;
    vt->screen_reverse = FALSE;

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            vt->screen[y][x] = (x == 80) ? '\0' : ' ';
        }

        vt->lines[y].bDirty = TRUE; /* Force an initial redraw */
        vt->lines[y].weight = kLineNormal;
        vt->lines[y].colstyle = (ColStyle*)malloc(sizeof(ColStyle));
        vt->lines[y].colstyle->next = NULL;
        vt->lines[y].colstyle->style = 0x00000700;
    }

    return e;
}

//EMULATOR_INIT_PLUGIN(vt100_init)

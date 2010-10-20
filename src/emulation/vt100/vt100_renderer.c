/**
 * @filename vt100_parser.c
 * @author Darryl Pogue & Terence Stenvold
 * @designer Darryl Pogue
 * @date 2010 10 18
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the helper functions for drawing data to the screen.
 */
#include "vt100.h"

/**
 * Parses the VT100 colour ids and returns a COLORREF.
 *
 * @param DWORD style   The line style.
 * @param BOOL fg       Returns the foreground colour if true, returns the
 *                      background colour otherwise.
 * @returns A COLORREF structure for the foreground or background colour.
 */
COLORREF parse_colour(DWORD style, BOOL fg) {
    CHAR c;

    if (fg) {
        c = (style & 0xFF00) >> 8;
    } else {
        c = (style & 0xFF);
    }

    switch(c) {
    case 0:
        return RGB(0, 0, 0);
    case 1:
        return RGB(255, 0, 0);
    case 2:
        return RGB(0, 255, 0);
    case 3:
        return RGB(255, 255, 0);
    case 4:
        return RGB(0, 0, 255);
    case 5:
        return RGB(255, 0, 255);
    case 6:
        return RGB(0, 255, 255);
    case 7:
        return RGB(255, 255, 255);
    default:
        if (fg) {
            return RGB(255, 255, 255);
        } else {
            return RGB(0, 0, 0);
        }
    }
}

/**
 * Draws a line of text to the screen.
 *
 * @param DWORD ln          The line number to be drawn.
 * @param VT100_Data* vt    The emulation mode data
 * @param HWND hwnd         The handle to the application window
 * @param HDC hdc           The handle to the device context.
 *                          If it is NULL, GetDC will be called.
 * @return 0 if successful, greater than 0 otherwise.
 */
DWORD draw_line(DWORD ln, VT100_Data* vt, HWND hwnd, HDC hdc) {
    LOGFONT lf;
    BOOL bGotDC = FALSE;
    HFONT font;
    TEXTMETRIC tm;
    BOOL changes = FALSE;
    DWORD x = 0;
    SIZE sz;
    ColStyle* current = vt->lines[ln].colstyle;
    ColStyle* next = current->next;

    if (hdc == NULL) {
        hdc = GetDC(hwnd);
        bGotDC = TRUE;
    }

    SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
    GetTextMetrics(hdc, &tm);
    GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);

    /* Enter pain and suffering */
    while (current != NULL) {
        INT start = (STYLE_POS(current->style) == 0 ? 0 : STYLE_POS(current->style) - 1);
        INT end = (next == NULL ? 80 : STYLE_POS(next->style) - 1);
        INT len = end - start;
        TCHAR* text = (TCHAR*)malloc(sizeof(TCHAR) * (len + 1));

        if (current->style & (kLineStyleBold << 16)) {
            lf.lfWeight = FW_BOLD;
            SetTextCharacterExtra(hdc, 0);
        } else {
            lf.lfWeight = FW_NORMAL;
            SetTextCharacterExtra(hdc, 1);
        }

        lf.lfUnderline = (current->style & (kLineStyleUnderline << 16)) ? TRUE : FALSE;

        font = CreateFontIndirect(&lf);
        SelectObject(hdc, font);

        _tcsncpy(text, vt->screen[ln] + start, len);
        text[len + 1] = 0;

        if (((current->style & (kLineStyleReverse << 16)) != 0) ^ (vt->screen_reverse)) {
            SetBkColor(hdc, parse_colour(current->style, TRUE));
            SetTextColor(hdc, parse_colour(current->style, FALSE));
        } else {
            SetBkColor(hdc, parse_colour(current->style, FALSE));
            SetTextColor(hdc, parse_colour(current->style, TRUE));
        }

        TextOut(hdc, x, ln * (tm.tmExternalLeading + tm.tmHeight), text, len);
        GetTextExtentPoint32(hdc, text, len, &sz);

        x += sz.cx;

        DeleteObject(font);

        current = next;
        if (current != NULL) {
            next = current->next;
        }
    }

    if (bGotDC) {
        ReleaseDC(hwnd, hdc);
    }

    vt->lines[ln].bDirty = FALSE;

    return 0;
}

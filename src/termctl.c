/**
 * @filename termctl.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the code for the terminal window "control".
 */
#include "termctl_private.h"

/**
 * Initialise the terminal data structure.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return none
 */
VOID InitTerminalCtl(HWND hwnd) {
    CtlData* data = (CtlData*)malloc(sizeof(CtlData));
    POINT origin = {0, 0};
    POINT max = {132, 25};
    POINT min = {80, 25};
    LOGFONT font;

    /* Set the CtlData as the terminal WndExtra */
    SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG)data);

    data->bConnected = FALSE;
    data->bScroll = TRUE;
    data->bWrap = TRUE;
    data->bEchoInput = TRUE;

    /* Set the cursor to a starting position */
    data->cursor.dwStyle = 0;
    data->cursor.pPosition = origin;

    /* Set up the font to use the default ANSI monospaced font */
    GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &font);
    SetTerminalFont(hwnd, font);

    data->pMaxSize = max;
    data->pMinSize = min;

    /* Initialise the lines with blank data */
    InitTerminalLines(data);
}

/**
 * Initialise the terminal lines.
 *
 * @param CtlData* data     The terminal control data.
 * @return none
 */
VOID InitTerminalLines(CtlData* data) {

    DWORD i = 0;

    /* Number of lines is equal to the maximum number we can fit */
    data->dwNumLines = data->pMaxSize.y;
    data->lpLines = (CtlTextLine**)malloc(sizeof(CtlTextLine*) * data->dwNumLines);

    for (i = 0; i < data->dwNumLines; i++) {
        /* Make each line NULL */
        data->lpLines[i] = NULL;
    }
    /* Initialise the first line for actual data */
    MakeTerminalLine(data, &(data->lpLines[0]));
}

/**
 * Sets the terminal font.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param LOGFONT font  The logical font to set.
 * @return none
 */
VOID SetTerminalFont(HWND hwnd, LOGFONT font) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    HDC hdc = GetDC(hwnd);
    TEXTMETRIC tm;

    /* Select the font so that we can get the text metrics */
    SelectObject(hdc, CreateFontIndirect(&font));
    GetTextMetrics(hdc, &tm);

    data->font = font;
    data->tm = tm;
}

/**
 * Sets the emulation mode for the terminal.
 *
 * @param HWND hwnd             The handle to the terminal window.
 * @param Emulator* emulator    The emulation mode to use.
 * @return none
 */
VOID SetTerminalEmulator(HWND hwnd, Emulator* emulator) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    data->lpEmulator = emulator;
}

/** 
 * Gets the terminal width in number of characters.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return The window width in characters.
 */
DWORD GetTerminalWidth(HWND hwnd) {
    RECT client;
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    DWORD width;

    GetClientRect(hwnd, &client);

    /* Characters per line is equal to the width of the window
       divided by the maximum width of a single character */
    width = (DWORD)(client.right / data->tm.tmMaxCharWidth);

    return width;
}

/** 
 * Gets the terminal height in number of characters.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return The window height in characters.
 */
DWORD GetTerminalHeight(HWND hwnd) {
    RECT client;
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    DWORD height;

    GetClientRect(hwnd, &client);

    /* Number of lines is equal to the height of the window
       divided by the height of 1 line */
    height = (DWORD)(client.bottom / (data->tm.tmHeight + data->tm.tmExternalLeading));

    return height;
}

POINT GetTerminalMinSize(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->pMinSize;
}

VOID SetTerminalMinSize(HWND hwnd, POINT size) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    data->pMinSize = size;
}

POINT GetTerminalMaxSize(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->pMaxSize;
}

VOID SetTerminalMaxSize(HWND hwnd, POINT size) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    data->pMaxSize = size;
}

/**
 * Gets whether the terminal has a scrollbar and is able to scroll up through
 * previous lines of text.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @returns true if there is a scrollbar, false otherwise.
 */
BOOLEAN GetTerminalScroll(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->bScroll;
}

/**
 * Gets whether the terminal will wrap text when it reaches the end of a line.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @returns true if text with automatically wrap, false otherwise.
 */
BOOLEAN GetTerminalWrap(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->bWrap;
}

/**
 * Gets whether the terminal will echo back characters as they are typed.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @returns true if characters will be echoed, false otherwise.
 */
BOOLEAN GetTerminalEcho(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->bEchoInput;
}

VOID SetTerminalProperties(HWND hwnd, BOOLEAN scroll, BOOLEAN wrap, BOOLEAN echo) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    data->bScroll = scroll;
    data->bWrap = wrap;
    data->bEchoInput = echo;
}

CtlCursor GetTerminalCursor(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    return data->cursor;
}

VOID SetTerminalCursorPos(HWND hwnd, DWORD x, DWORD y) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    POINT pos = {x, y};

    data->cursor.pPosition = pos;
}

VOID SetTerminalCursorStyle(HWND hwnd, DWORD dwStyle) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    data->cursor.dwStyle = dwStyle;
}

/**
 * Adds text at the current position in the terminal.
 *
 * @param HWND hwnd         The handle to the terminal window.
 * @param TCHAR* lpszText   The text to be added.
 * @return none
 */
VOID AddTerminalText(HWND hwnd, TCHAR* lpszText) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    TCHAR* buf = data->lpLines[data->cursor.pPosition.y]->lpszText;
    size_t strSize;
    size_t remaining;

    /* The line size is the max size minus the current X position */
    DWORD lineSize = data->pMaxSize.x - data->cursor.pPosition.x;

    StringCchLength(lpszText, data->pMaxSize.x, &strSize);

    /* Move horizontally by the cursor X value */
    buf += data->cursor.pPosition.x;

    /* Copy the text from lpszText to the line */
    StringCchCopyEx(buf, lineSize + 1, lpszText, NULL, &remaining, STRSAFE_IGNORE_NULLS);
    data->cursor.pPosition.x += lineSize - (--remaining); /* We added 1 for the NULL, so subtract that now */

    if (remaining == 0) {
        if (strSize > lineSize) {
            if (data->bWrap) {
                /* Print the rest of the text on the next line */
                data->cursor.pPosition.y++;
            } else {
                buf[lineSize] = lpszText[strSize - 1];
            }
        } else {
            /* Move the cursor down */
            data->cursor.pPosition.y++;
            data->cursor.pPosition.x = 0;

            if (data->cursor.pPosition.y >= data->dwNumLines
                    || data->lpLines[data->cursor.pPosition.y] == NULL) {
                AddTerminalLine(hwnd, TEXT(""));
            }
        }
    }

    PaintTerminalLine(hwnd, GetDC(hwnd), data->cursor.pPosition.y);
}

/**
 * Create a new line on the terminal.
 *
 * @param CtlData* data         The control data structure.
 * @param CtlTextLine** line    The line to be created.
 * @return none
 */
VOID MakeTerminalLine(CtlData* data, CtlTextLine** line) {
    *line = (CtlTextLine*)malloc(sizeof(CtlTextLine));

    (*line)->lpszText = (TCHAR*)malloc(sizeof(TCHAR) * (data->pMaxSize.x + 1));
    (*line)->lpdwStyle = (DWORD*)malloc(sizeof(DWORD) * data->pMaxSize.x);

    memset((*line)->lpszText, '\0', data->pMaxSize.x);
    memset((*line)->lpdwStyle, 0, data->pMaxSize.x);
}

/**
 * Adds a line of text to the terminal.
 *
 * @param HWND hwnd         The handle to the terminal window.
 * @param TCHAR* lpszText   The text to be added.
 * @return none
 */
VOID AddTerminalLine(HWND hwnd, TCHAR* lpszText) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    DWORD i = 0;

    /* Try to find the next available line */
    for (i = 0; i < data->dwNumLines; i++) {
        if (data->lpLines[i] == NULL)
            break;
    }

    /* Check if we need to add a new line */
    if (i == data->dwNumLines) {
        if (data->bScroll) {
            /* If we're scrolling, add a new line at the bottom */
            CtlTextLine** tmp = data->lpLines;
            data->lpLines = (CtlTextLine**)malloc(sizeof(CtlTextLine*) * ++(data->dwNumLines));
            for (i = 0; i < data->dwNumLines - 1; i++) {
                data->lpLines[i] = tmp[i];
            }
        } else {
            /* Otherwise, shuffle the rest of the text up */
            for (i = 0; i < data->dwNumLines - 1; i++) {
                data->lpLines[i] = data->lpLines[i + 1];
            }
        }
    }

    MakeTerminalLine(data, &(data->lpLines[i]));
    SetTerminalCursorPos(hwnd, 0, i);
    AddTerminalText(hwnd, lpszText);
}

/**
 * Paint the entire terminal window.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param HDC hdc       The handle to the device context.
 * @return none
 */
VOID PaintTerminal(HWND hwnd, HDC hdc) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    DWORD i = 0;

    for (i = 0; i < data->dwNumLines; i++) {
        if (data->lpLines[i] != NULL) {
            PaintTerminalLine(hwnd, hdc, i);
        }
    }
}

/**
 * Paint a line of the terminal.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param HDC hdc       The handle to the device context.
 * @param DWORD line    The line to be painted.
 * @return none
 */
VOID PaintTerminalLine(HWND hwnd, HDC hdc, DWORD line) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    TCHAR* text = data->lpLines[line]->lpszText;
    SCROLLINFO si;
    INT xpos;
    INT ypos;
    size_t line_size;
    INT i;
    SIZE sz;
    DWORD x = 2;

    /* Set the font for the line of text */
    SelectObject(hdc, CreateFontIndirect(&data->font));
    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));

    /* Get the Y offset from the scrollbar */
    GetScrollInfo(hwnd, SB_VERT, &si);
    ypos = si.nPos;

    /* Get the X offset from the scrollbar */
    GetScrollInfo(hwnd, SB_HORZ, &si);
    xpos = si.nPos;

    /* Get the string length, and draw each character */
    StringCchLength(text, data->pMaxSize.x + 1, &line_size);
    for (i = 0; i < line_size; i++) {
        /* NOTE:
         * The reason that we are drawing one character at a time is to allow
         * for style changes such as bold, underline, blinking, and alternate
         * foreground/background colours.
         * At the moment, we assume the default style (white-on-black) is used.
         */
        TextOut(hdc, x - xpos, (line * (data->tm.tmExternalLeading + data->tm.tmHeight)) - ypos + 2, (LPCTSTR)&text[i], 1);
        GetTextExtentPoint32(hdc, (LPCTSTR)&text[i], 1, &sz);

        x += sz.cx;
    }
}

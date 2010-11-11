/**
 * @filename vt100_parser.c
 * @author Darryl Pogue & Terence Stenvold
 * @designer Darryl Pogue
 * @date 2010 10 18
 * @project Terminal Emulator: VT100 Plugin
 *
 * This file contains the helper functions for parsing the VT100 escape 
 * sequences.
 */
#include "vt100.h"

/**
 * Frees the nodes of a linked list recursively.
 *
 * @param ColStyle* root    The root of the linked list.
 * @returns none
 */
void free_tree_recur(ColStyle* root) {
    if (root->next != NULL) {
        free_tree_recur(root->next);
        root->next = NULL;
    }
    free(root);
    root = NULL;
}

/**
 * Sets the style of a line on the screen.
 *
 * @param ColStyle* ln  Pointer to the linked list of style information
 *                      for the line.
 * @param DWORD x       The column number at which to set the style.
 * @param DWORD style   The style to be set.
 * @returns none
 */
void set_line_style(ColStyle* ln, DWORD x, DWORD style) {
    ColStyle* head = ln;
    ColStyle* cur = ln;

    if (x == 0) {
        if (cur->next != NULL) {
            free_tree_recur(cur->next);
        }
        cur->next = NULL;
        cur->style = style;
        return;
    }

    for (cur = head; cur != NULL; cur = cur->next) {
        if (cur->next == NULL) {
            if ((cur->style & 0x00FFFFFF) != style) {
                ColStyle* newnext = (ColStyle*)malloc(sizeof(ColStyle));
                newnext->style = (x << 24) | style;
                if (x < 80) {
                    newnext->next = (ColStyle*)malloc(sizeof(ColStyle));
                    newnext->next->style = ((x + 1) << 24) | 0x000700;
                    newnext->next->next = NULL;
                } else {
                    newnext->next = NULL;
                }
                cur->next = newnext;
            }
            break;
        } else if (STYLE_POS(cur->next->style) > x) {
            /* Replace the rest of the list */
            if (cur->next->next != NULL) {
                free_tree_recur(cur->next->next);
            }
            cur->next->style = (x << 24) | style;
            if (x < 80) {
                cur->next->next = (ColStyle*)malloc(sizeof(ColStyle));
                cur->next->next->style = ((x + 1) << 24) | 0x000700;
                cur->next->next->next = NULL;
            } else {
                cur->next->next = NULL;
            }
            break;
        } else if (STYLE_POS(cur->next->style) == x) {
            if (cur->next->next != NULL) {
                free_tree_recur(cur->next->next);
            }
            cur->next->style = (x << 24) | style;
            if (x < 80) {
                cur->next->next = (ColStyle*)malloc(sizeof(ColStyle));
                cur->next->next->style = ((x + 1) << 24) | 0x000700;
                cur->next->next->next = NULL;
            } else {
                cur->next->next = NULL;
            }
            break;
        }
    }
}

/**
 * Sets the cursor style.
 *
 * @param VT100_Data* vt    The emulator mode data
 * @param CHAR attr         The style attributes the set.
 * @param CHAR fg           The foreground colour
 * @param CHAR bg           The background colour
 * @param BOOL reset        Reset styles if true
 * @returns none
 */
void set_style(VT100_Data* vt, CHAR attr, CHAR fg, CHAR bg, BOOL reset) {
    vt->current.style |= (attr << 16);

    if (fg) {
        vt->current.style &= 0xFFFF00FF;
        vt->current.style |= ((fg - 30) << 8);
    }

    if (bg) {
        vt->current.style &= 0xFFFFFF00;
        vt->current.style |= (bg - 40);
    }

    if (reset) {
        vt->current.style = 0x00000700;
    }
}

/**
 * Handles an escape sequence beginning with ESC[ and ending with m.
 *
 * @param LPCTSTR rx        The escape sequence (ESC[ is trimmed)
 * @param VT100_Data* vt    The emulation mode data
 * @returns none
 */
void escape_colour(LPCTSTR rx, VT100_Data* vt) {
    DWORD parsedtoks = 0;

    if (rx[0] == ';') {
        set_style(vt, 0, 0, 0, TRUE);
        ++rx;
    }

    while(_tcsclen(rx) != 0) {
        if (isdigit(rx[0])) {
            INT d = -1;
            INT tokens = _stscanf(rx, TEXT("%d"), &d);

            if (tokens == 0) { /* Should be impossible */
                ++rx;
                continue;
            }

            parsedtoks++;
            switch (d) {
            case 0:
                set_style(vt, 0, 0, 0, TRUE);
                break;
            case 1:
                set_style(vt, kLineStyleBold, 0, 0, FALSE);
                break;
            case 4:
                set_style(vt, kLineStyleUnderline, 0, 0, FALSE);
                break;
            case 5:
                set_style(vt, kLineStyleBlink, 0, 0, FALSE);
                break;
            case 7:
                set_style(vt, kLineStyleReverse, 0, 0, FALSE);
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
                set_style(vt, 0, (CHAR)d, 0, FALSE);
                break;
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
                set_style(vt, 0, 0, (CHAR)d, FALSE);
                break;
            default:
                break;
            }

            ++rx;
            while (isdigit(rx[0])) {
                ++rx;
            }
        } else {
            ++rx;
        }
    }

    if (parsedtoks == 0) {
        set_style(vt, 0, 0, 0, TRUE);
    }
}

/**
 * Handles an escape sequence beginning with ESC[.
 *
 * @param LPCTSTR rx        The escape sequence (ESC[ is trimmed)
 * @param VT100_Data* vt    The emulation mode data
 * @returns none
 */
void escape_bracket(LPCTSTR rx, VT100_Data* vt) {
    DWORD num1 = 0;
    DWORD num2 = 0;
    TCHAR chr1;
    TCHAR chr2;
    int tokens;

    if (rx[0] == '?') {
        escape_question(rx + 1, vt);
        return;
    }

    tokens = _stscanf(rx, TEXT("%d%c%d%c"), &num1, &chr1, &num2, &chr2);

    if (!tokens) {
        if (isalpha(rx[0])) {
            chr1 = rx[0];
            tokens = 2;
            if (rx[0] == 'r' || rx[0] == 'H' || rx[0] == 'f') {
                chr2 = rx[0];
                tokens = 4;
            }
        } else if (rx[0] == ';' && (isalpha(rx[1]) || isdigit(rx[1]) || rx[1] == ';')) {
            chr1 = rx[0];
            chr2 = rx[1];
            tokens = 4;
        }
    }

    if (tokens == 4) {
        switch (chr2) {
        case 'H':
        case 'f':
            {
                vt->current.x = (num2 == 0 ? 0 : num2 - 1) + vt->origin.x;
                vt->current.y = (num1 == 0 ? 0 : num1 - 1) + vt->origin.y;

                if (vt->current.x >= 80) {
                    if (vt->autowrap) {
                        vt->current.x = 0;
                        vt->current.y += 1;
                    } else {
                        vt->current.x = 79;
                    }
                }
                if (vt->current.y >= 24) {
                    if (vt->autowrap) {
                            scroll_screen(1, vt);
                    }

                    vt->current.y = 23;
                }
            }
            break;
        case 'r':
            {
                vt->scroll_top = (num1 == 0 ? 0 : num1 - 1);
                vt->scroll_bottom = (num2 == 0 ? 23 : num2 - 1);

                if (vt->relorigin) {
                    vt->origin.y = vt->scroll_top;
                }

                vt->current.x = vt->origin.x;
                vt->current.y = vt->origin.y;
            }
            break;
        }
        return;
    } else if (tokens == 2) {
        switch (chr1) {
        case 'A':
            {
                num1 = (num1 == 0 ? 1 : num1);
                if (vt->current.y > num1) {
                    vt->current.y -= num1;
                } else {
                    vt->current.y = 0;
                }
            }
            break;
        case 'B':
            {
                vt->current.y += (num1 == 0 ? 1 : num1);
                if (vt->current.y > 23)
                    vt->current.y = 23;
            }
            break;
        case 'C':
            {
                vt->current.x += (num1 == 0 ? 1 : num1);
                if (vt->current.x > 79)
                    vt->current.x = 79;
            }
            break;
        case 'D':
            {
                num1 = (num1 == 0 ? 1 : num1);
                if (vt->current.x > num1) {
                    vt->current.x -= num1;
                } else {
                    vt->current.x = 0;
                }
            }
            break;
        case 'J':
            {
                DWORD y;
                DWORD x;
                switch(num1) {
                case 0:
                    {
                        for (y = vt->current.y; y < 24; y++) {
                            set_style(vt, 0, 0, 0, TRUE);
                            set_line_style(vt->lines[y].colstyle, 0, 0x00000700);

                            for (x = 0; x < 80; x++) {
                                vt->screen[y][x] = ' ';
                            }
                            vt->lines[y].weight = kLineNormal;
                            vt->lines[y].bDirty = TRUE;
                        }
                    }
                    break;
                case 1:
                    {
                        for (y = 0; y < vt->current.y; y++) {
                            set_style(vt, 0, 0, 0, TRUE);
                            set_line_style(vt->lines[y].colstyle, 0, 0x00000700);
                            for (x = 0; x < 80; x++) {
                                vt->screen[y][x] = ' ';
                            }
                            vt->lines[y].weight = kLineNormal;
                            vt->lines[y].bDirty = TRUE;
                        }
                        for (x = 0; x <= vt->current.x; x++) {
                            vt->screen[y][x] = ' ';
                        }
                    }
                    break;
                case 2:
                    {
                        for (y = 0; y < 24; y++) {
                            set_style(vt, 0, 0, 0, TRUE);
                            set_line_style(vt->lines[y].colstyle, 0, 0x00000700);
                            for (x = 0; x < 80; x++) {
                                vt->screen[y][x] = ' ';
                            }
                            vt->lines[y].weight = kLineNormal;
                            vt->lines[y].bDirty = TRUE;
                        }
                    }
                    break;
                }
            }
            break;
        case 'K':
            {
                DWORD x;

                vt->lines[vt->current.y].bDirty = TRUE;
                switch(num1) {
                case 0:
                    {
                        set_style(vt, 0, 0, 0, TRUE);
                        set_line_style(vt->lines[vt->current.y].colstyle, vt->current.x, 0x00000700);

                        for (x = vt->current.x; x < 80; x++) {
                            vt->screen[vt->current.y][x] = ' ';
                        }
                    }
                    break;
                case 1:
                    {
                        for (x = 0; x <= vt->current.x; x++) {
                            vt->screen[vt->current.y][x] = ' ';
                        }
                    }
                    break;
                case 2:
                    {
                        set_style(vt, 0, 0, 0, TRUE);
                        set_line_style(vt->lines[vt->current.y].colstyle, vt->current.x, 0x00000700);

                        for (x = 0; x < 80; x++) {
                            vt->screen[vt->current.y][x] = ' ';
                        }
                    }
                    break;
                }
            }
            break;
        case 'g':
            {
                switch(num1) {
                case 0:
                    vt->htabs[vt->current.x] = 0;
                    break;
                case 3:
                    {
                        DWORD x;
                        for (x = 0; x < 80; x++) {
                            vt->htabs[x] = 0;
                        }
                        vt->htabs[79] = 1;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case 'h':
            escape_question(rx + 1, vt);
            break;
        case 'l':
            escape_question(rx + 1, vt);
            break;
        }
        return;
    }

    OutputDebugString(TEXT("\tUnhandled!"));
}

/**
 * Handles an escape sequence beginning with ESC?.
 *
 * @param LPCTSTR rx        The escape sequence (ESC? is trimmed)
 * @param VT100_Data* vt    The emulation mode data
 * @returns none
 */
void escape_question(LPCTSTR rx, VT100_Data* vt) {
    BOOL set = (rx[1] == 'h');

    switch (rx[0]) {
    case '1':
        {
            if (set) {
                vt->appcursormode |= kCursorApplicationMode;
            } else {
                vt->appcursormode &= ~kCursorApplicationMode;
            }
        }
        break;
    case '3':
        {
            /* We don't actually handle 132 column mode */
            /* but this command clears the screen */

            DWORD y;
            DWORD x;

            for (y = 0; y < 24; y++) {
                    set_style(vt, 0, 0, 0, TRUE);
                    set_line_style(vt->lines[y].colstyle, 0, 0x00000700);
                for (x = 0; x < 80; x++) {
                    vt->screen[y][x] = ' ';
                }
                vt->lines[y].weight = kLineNormal;
                vt->lines[y].bDirty = TRUE;
            }
        }
        break;
    case '4':
        /* Smooth Scrolling is not implemented */
        break;
    case '5':
        {
            if (vt->screen_reverse != set) {
                DWORD y = 0;

                vt->screen_reverse = set;

                for (y = 0; y < 24; y++) {
                    vt->lines[y].bDirty = TRUE;
                }
            }
        }
        break;
    case '6':
        {
            vt->relorigin = set;

            if (set) {
                vt->origin.y = vt->scroll_top;
            } else {
                vt->origin.x = 0;
                vt->origin.y = 0;
            }
            vt->current.x = vt->origin.x;
            vt->current.y = vt->origin.y;
        }
        break;
    case '7':
        {
            if (set) {
                vt->autowrap = TRUE;
            } else {
                vt->autowrap = FALSE;
            }
        }
        break;
    default:
        OutputDebugString(TEXT("\tUnhandled!"));
        break;
    }
}

/**
 * Handles an escape sequence beginning with ESC#.
 *
 * @param LPCTSTR rx        The escape sequence (ESC# is trimmed)
 * @param VT100_Data* vt    The emulation mode data
 * @returns none
 */
void escape_hash(LPCTSTR rx, VT100_Data* vt) {
    switch(rx[0]) {
    case '3':
        {
            vt->lines[vt->current.y].weight &= ~kLineDoubleBottom;
            vt->lines[vt->current.y].weight |= kLineDoubleTop;
            vt->lines[vt->current.y].weight |= kLineDoubleWidth;
            vt->lines[vt->current.y].bDirty = TRUE;
        }
        break;
    case '4':
        {
            vt->lines[vt->current.y].weight &= ~kLineDoubleTop;
            vt->lines[vt->current.y].weight |= kLineDoubleBottom;
            vt->lines[vt->current.y].weight |= kLineDoubleWidth;
            vt->lines[vt->current.y].bDirty = TRUE;
        }
        break;
    case '5':
        {
            vt->lines[vt->current.y].weight = kLineNormal;
            vt->lines[vt->current.y].bDirty = TRUE;
        }
        break;
    case '6':
        {
            vt->lines[vt->current.y].weight |= kLineDoubleWidth;
            vt->lines[vt->current.y].bDirty = TRUE;
        }
        break;
    case '8':
        {
            for (vt->current.y = 0;
                    vt->current.y < 24;
                    vt->current.y += 1) {
                for (vt->current.x = 0;
                        vt->current.x < 80;
                        vt->current.x += 1) {
                    vt->screen[vt->current.y][vt->current.x] = 'E';
                }
                vt->lines[vt->current.y].bDirty = TRUE;
            }

            vt->current.x = 0;
            vt->current.y = 0;
        }
        break;
    default:
        OutputDebugString(TEXT("\tUnhandled!"));
        break;
    }
}

/**
 * Scrolls all the lines in the scrolling region up or down by one line.
 *
 * @param CHAR up           Scroll up if greater than 0, down otherwise
 * @param VT100_Data* vt    The emulation mode data
 * @return none
 */
void scroll_screen(char up, VT100_Data* vt) {
    DWORD x;
    DWORD y;

    if (up) {
        for (y = vt->scroll_top; y < vt->scroll_bottom; y++) {
            TCHAR* below = vt->screen[(y+1)];
            _tcsncpy(vt->screen[y], below, 80);
            vt->lines[y].bDirty = TRUE;

            vt->lines[y].colstyle = vt->lines[y+1].colstyle;
        }
    } else {
        for (y = vt->scroll_bottom; y > vt->scroll_top; y--) {
            TCHAR* above = vt->screen[(y-1)];
            _tcsncpy(vt->screen[y], above, 80);
            vt->lines[y].bDirty = TRUE;

            vt->lines[y].colstyle = vt->lines[y-1].colstyle;
        }
    }

    vt->lines[y].colstyle = (ColStyle*)malloc(sizeof(ColStyle));
    vt->lines[y].colstyle->next = NULL;
    vt->lines[y].colstyle->style = 0x00000700;

    for (x = 0; x < 80; x++) {
        vt->screen[y][x] = ' ';
    }
}

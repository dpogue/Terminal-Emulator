/**
 * @filename terminal_win.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the implementations of all WIN32 functions for the
 * terminal window.
 */
#include "terminal.h"

/**
 * The application entry point.
 *
 * @param HINSTANCE hInstance       The handle to the application instance
 * @param HINSTANCE hPrevInstance   The handle to the previous instance
 * @param LPSTR lspszCmdParam       The command line arguments to the app
 * @param int iCmdShow              The default window display state
 * @returns zero on success, non-zero on failure
 */
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hprevInstance,
                          LPSTR lspszCmdParam, int iCmdShow) {
    HWND hwnd;

    if (RegisterWindowClass(hInstance) != 0) {
        return 1;
    }

    if ((hwnd = CreateTerminal(hInstance)) == NULL) {
        return 2;
    }

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    MessageLoop(hwnd);
    return 0;
}

/**
 * Registers the window class with the system.
 *
 * @param HINSTANCE hInstance       The handle to the application instance
 * @returns zero on success, non-zero on failure
 */
DWORD RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASS wndclass;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = sizeof(TerminalModes*);
    wndclass.cbWndExtra    = sizeof(CtlData*);
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wndclass.lpszMenuName  = TEXT("TerminalMenu");
    wndclass.lpszClassName = APPNAME;

    if (!RegisterClass (&wndclass))
    {
        MessageBox(NULL, TEXT("The application was unable to run"), 
                      APPNAME, MB_ICONERROR);
        return 1;
    }

    return 0;
}

/**
 * Creates the terminal window and initialises the instance data.
 *
 * @param HINSTANCE hInstance       The handle to the application instance
 * @return The handle to the terminal window.
 */
HWND CreateTerminal(HINSTANCE hInstance) {
    RECT rect;
    TerminalModes* modes;
    HWND terminal = CreateWindow(APPNAME, APPNAME,
                         (WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL) & ~WS_MAXIMIZEBOX,
                         CW_USEDEFAULT, CW_USEDEFAULT, 500, 100,
                         NULL, NULL, hInstance, NULL);

    InitTerminalCtl(terminal);

    /* Load the plugins */
    LoadPlugins(terminal);
    modes = (TerminalModes*)GetClassLongPtr(terminal, 0);
    modes->hPrefs = CreatePreferencesDlg(terminal);

    /* The default mode is No Emulation */
    SetTerminalEmulator(terminal, modes->lpEmulators[0]);

    /* This will force the window to re-evaluate its min/max size and resize */
    GetWindowRect(terminal, &rect);
    SetWindowPos(terminal, 0, rect.left, rect.top, 1, 1, 0);

    /* We start with no open connections */
    Disconnect(terminal);

    return terminal;
}

/**
 * Pull messages off of the queue and send them for parsing.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return none
 */
VOID MessageLoop(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    TerminalModes* modes = (TerminalModes*)GetClassLongPtr(hwnd, 0);
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        /* Check if the emulation plugin has overridden the wndproc */
        if (EMULATOR_HAS_FUNC(data->lpEmulator, wnd_proc_override)) {
            if (data->lpEmulator->wnd_proc_override(data->lpEmulator->emulator_data, &msg)) {
                continue;
            }
        }

        /* Check if this is a dialog message */
        if (!IsWindow(modes->hPrefs) || !IsDialogMessage(modes->hPrefs, &msg)) {
            /* Otherwise, process it as usual */
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
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
LRESULT CALLBACK WndProc (HWND hwnd, UINT message,
                          WPARAM wParam, LPARAM lParam) {
    switch(message) {
    HANDLE_MSG(hwnd, WM_PAINT, Terminal_OnPaint);
    HANDLE_MSG(hwnd, WM_GETMINMAXINFO, Terminal_OnGetMinMaxInfo);
    HANDLE_MSG(hwnd, WM_SIZE, Terminal_OnSize);
    HANDLE_MSG(hwnd, WM_COMMAND, Terminal_OnCommand);
    HANDLE_MSG(hwnd, WM_HSCROLL, Terminal_OnHScroll);
    HANDLE_MSG(hwnd, WM_VSCROLL, Terminal_OnVScroll);
    HANDLE_MSG(hwnd, WM_KEYUP, Terminal_OnKey);
    HANDLE_MSG(hwnd, WM_DESTROY, Terminal_OnDestroy);
    case WM_CHAR:
        /* According to wine's source code:
           We can't use HANDLE_MSG with WM_CHAR because it will truncate unicode values */
        Terminal_OnChar(hwnd, (TCHAR)wParam);
        return 0;
    case TWM_DISCONNECT:
        Disconnect(hwnd);
        return 0;
    default:
        return DefWindowProc (hwnd, message, wParam, lParam);
    }
}

/**
 * Message handler for the WM_PAINT message.
 *
 * @param HWND hwnd             The handle to the terminal window.
 * @return none
 */
VOID Terminal_OnPaint(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    HDC hdc;
    PAINTSTRUCT ps;

    hdc = BeginPaint(hwnd, &ps);

    if (data->bConnected) {
        PaintTerminal(hwnd, hdc);
    } else {
        HBRUSH bg = CreateSolidBrush(RGB(64, 64, 64));
        RECT r;
        GetClientRect(hwnd, &r);

        FillRect(hdc, &r, bg);
        DeleteObject(bg);
    }

    EndPaint(hwnd, &ps);
}

/**
 * Message handler for the WM_GETMINMAXINFO message.
 *
 * @param HWND hwnd             The handle to the terminal window.
 * @param LPMINMAXINFO lpInfo   The MINMAXINFO structure.
 * @return none
 */
VOID Terminal_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpInfo) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    DWORD minY;
    DWORD minX;
    DWORD maxY;
    DWORD maxX;

    if (data == NULL)
        return;

    /* Calculate the minimum window width */
    minX = data->pMinSize.x * data->tm.tmMaxCharWidth;
    minX += GetSystemMetrics(SM_CXHSCROLL);
    minX += (GetSystemMetrics(SM_CXFRAME) + 3) * 2;

    /* Calculate the minimum window height */
    minY = data->pMinSize.y * (data->tm.tmHeight + data->tm.tmExternalLeading);
    minY += GetSystemMetrics(SM_CYMENU);
    minY += GetSystemMetrics(SM_CYCAPTION);
    minY += GetSystemMetrics(SM_CYVSCROLL);

    /* Calculate the maximum window width */
    maxX = data->pMaxSize.x * data->tm.tmMaxCharWidth;
    maxX += GetSystemMetrics(SM_CXHSCROLL);
    maxX += (GetSystemMetrics(SM_CXFRAME) + 3) * 2;

    /* Calculate the maximum window height */
    maxY = data->pMaxSize.y * (data->tm.tmHeight + data->tm.tmExternalLeading);
    maxY += GetSystemMetrics(SM_CYMENU);
    maxY += GetSystemMetrics(SM_CYCAPTION);
    maxY += GetSystemMetrics(SM_CYVSCROLL);

    /* Confine the window to that range of sizes */
    lpInfo->ptMinTrackSize.x = minX;
    lpInfo->ptMinTrackSize.y = minY;
    lpInfo->ptMaxTrackSize.x = maxX;
    lpInfo->ptMaxTrackSize.y = maxY;
}

/**
 * Message handler for the WM_SIZE message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param UINT state    Unused.
 * @param INT cx        The window width.
 * @param INT cy        The window height.
 * @return none
 */
VOID Terminal_OnSize(HWND hwnd, UINT state, INT cx, INT cy) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    SCROLLINFO si;
    DWORD height;
    DWORD width;

    if (data == NULL)
        return;

    /* If scrollbars are disabled, hide them */
    if (!data->bScroll) {
        ShowScrollBar(hwnd, SB_BOTH, FALSE);
        return;
    }

    height = (DWORD)(cy / (data->tm.tmHeight + data->tm.tmExternalLeading));
    width = (DWORD)(cx / data->tm.tmMaxCharWidth);

    /* Set up the vertical scrollbar */
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = data->dwNumLines - 1;
    si.nPage = height;
    si.nPos = 0;
    SetScrollInfo(hwnd, SB_VERT, &si, FALSE);

    /* Set up the horizontal scrollbar */
    si.nMax = data->pMaxSize.x;
    si.nPage = width;
    SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
}

/**
 * Message handler for the WM_COMMAND message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param INT id        The ID of the command item.
 * @param HWND ctl      The handle to the control.
 * @param UINT notify   Unused.
 * @return none
 */
VOID Terminal_OnCommand(HWND hwnd, INT id, HWND ctl, UINT notify) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    switch (id) {
    case ID_EXIT:
        DestroyWindow(hwnd);
        break;
    case ID_DISCONNECT:
        Disconnect(hwnd);
        break;
    case ID_CONNECT:
        Connect(hwnd);
        break;
    case ID_PREFERENCES:
        ShowPreferencesDlg(hwnd);
        break;
    case ID_EMU_START:
    case ID_EMU_ONE:
    case ID_EMU_TWO:
        {
            HMENU menubar = GetMenu(hwnd);
            TerminalModes* modes = (TerminalModes*)GetClassLongPtr(hwnd, 0);
            HWND prefs = modes->hPrefs;

            CheckMenuItem(menubar, ID_EMU_START, MF_UNCHECKED);
            Button_SetCheck(GetDlgItem(prefs, PREFERENCES_NONE), BST_UNCHECKED);
            CheckMenuItem(menubar, ID_EMU_ONE, MF_UNCHECKED);
            Button_SetCheck(GetDlgItem(prefs, PREFERENCES_ONE), BST_UNCHECKED);
            CheckMenuItem(menubar, ID_EMU_TWO, MF_UNCHECKED);
            Button_SetCheck(GetDlgItem(prefs, PREFERENCES_TWO), BST_UNCHECKED);

            CheckMenuItem(menubar, id, MF_CHECKED);
            Button_SetCheck(GetDlgItem(prefs, id - ID_EMU_START + PREFERENCES_NONE), BST_CHECKED);
        }
        break;
    }
}

/**
 * Message handler for the WM_HSCROLL message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param HWND hwndCtl  The handle to the control.
 * @param UINT code     The scroll event code.
 * @param INT pos       The scroll position.
 * @return none
 */
VOID Terminal_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, INT pos) {
    SCROLLINFO si;
    INT iHorzPos;

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;

    GetScrollInfo(hwnd, SB_HORZ, &si);
    iHorzPos = si.nPos;

    switch (code) {
        case SB_LEFT:
            si.nPos = si.nMin;
            break;
        case SB_RIGHT:
            si.nPos = si.nMax;
            break;
        case SB_LINELEFT:
            si.nPos--;
            break;
        case SB_LINERIGHT:
            si.nPos++;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        default:
            break;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
    GetScrollInfo(hwnd, SB_HORZ, &si);

    if (si.nPos != iHorzPos) {
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
}

/**
 * Message handler for the WM_VSCROLL message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param HWND hwndCtl  The handle to the control.
 * @param UINT code     The scroll event code.
 * @param INT pos       The scroll position.
 * @return none
 */
VOID Terminal_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, INT pos) {
    SCROLLINFO si;
    INT iVertPos;

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;

    GetScrollInfo(hwnd, SB_VERT, &si);
    iVertPos = si.nPos;

    switch (code) {
        case SB_TOP:
            si.nPos = si.nMin;
            break;
        case SB_BOTTOM:
            si.nPos = si.nMax;
            break;
        case SB_LINEUP:
            si.nPos--;
            break;
        case SB_LINEDOWN:
            si.nPos++;
            break;
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        default:
            break;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    GetScrollInfo(hwnd, SB_VERT, &si);

    if (si.nPos != iVertPos) {
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
}

/**
 * Message handler for the WM_KEYUP message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param UINT vk       The virtual key code of the input.
 * @param BOOL fDown    Unused.
 * @param INT repeat    Unused.
 * @param UINT flags    Unused.
 * @return none
 */
VOID Terminal_OnKey(HWND hwnd, UINT vk, BOOL fDown, INT repeat, UINT flags) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    if (EMULATOR_HAS_FUNC(data->lpEmulator, escape_input)) {
        LPCSTR send = data->lpEmulator->escape_input(data->lpEmulator->emulator_data, vk);
        size_t size_send;

        StringCchLength((LPCTSTR)send, 1024, &size_send);

        if (SendData(&data->hCommDev, (LPVOID)send, size_send) != 0) {
            DWORD dwError = GetLastError();
            ReportError(dwError);
        }
    }
}

/**
 * Message handler for the WM_CHAR message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @param TCHAR c       The character that was entered.
 * @return none
 */
VOID Terminal_OnChar(HWND hwnd, TCHAR c) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    if (data->bConnected && EMULATOR_HAS_FUNC(data->lpEmulator, escape_input)) {
        if (data->bEchoInput) {
            TCHAR text[2] = {c, '\0'};
            /* Print the character to the screen locally */
            AddTerminalText(hwnd, text);
        }

        /* Send it out the serial port */
        if (SendData(&data->hCommDev, (LPVOID)&c, sizeof(TCHAR)) != 0) {
            DWORD dwError = GetLastError();
            ReportError(dwError);
        }
    }
}

/**
 * Message handler for the WM_DESTROY message.
 *
 * @param HWND hwnd     The handle to the terminal window.
 * @return none
 */
VOID Terminal_OnDestroy(HWND hwnd) {
    CtlData* data = (CtlData*)GetWindowLongPtr(hwnd, GWL_USERDATA);

    Disconnect(hwnd);
    PostQuitMessage(0);
}

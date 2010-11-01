/**
 * @filename terminal_win.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 09 24
 * @project Terminal Emulator (COMP3980 Asn1)
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
    MSG msg;
    WNDCLASS wndclass;
    TermInfo* wndData;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = sizeof(TermInfo*);
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

    hwnd = CreateWindow(APPNAME, APPNAME,
                         WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX),
                         CW_USEDEFAULT, CW_USEDEFAULT, 500, 100,
                         NULL, NULL, hInstance, NULL);

    wndData = (TermInfo*)malloc(sizeof(TermInfo));
    wndData->dwMode = kModeCommand;
    wndData->hEmulator = rfid_init(hwnd);
    wndData->hwnd = hwnd;
    wndData->hReadLoop = NULL;
    SetWindowLongPtr(hwnd, 0, (LONG)wndData);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    CommandMode(hwnd);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
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
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);

    switch(message) {
    case WM_SIZE:
        {
            HDC hdc;
            TEXTMETRIC tm;
            HFONT monospace;
            DWORD height;
            DWORD width;

            hdc = GetDC(hwnd);
            monospace = (HFONT)GetStockObject(ANSI_FIXED_FONT);
            SelectObject(hdc, monospace);
            GetTextMetrics(hdc, &tm);

            width = (tm.tmMaxCharWidth * 81) + 78; /* Add 1 for bold padding */

            /* We need to take window borders and menu bars into account */
            height = (tm.tmExternalLeading + tm.tmHeight) * 25;
            height += GetSystemMetrics(SM_CYMENU);
            height += GetSystemMetrics(SM_CYCAPTION);
            height -= 2 * GetSystemMetrics(SM_CYFIXEDFRAME);

            SetWindowPos(hwnd, NULL, 100, 100, width, height, 0);
            ReleaseDC(hwnd, hdc);
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)){
        case ID_EXIT:
            DestroyWindow(hwnd);
            break;
        case ID_DISCONNECT:
            CommandMode(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        default:
            {
                DWORD port = LOWORD(wParam) - ID_COM_START;

                ConnectMode(hwnd, port);
            }
            break;
        }
        return 0;
    case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hwnd, &ps);
            if (ti->dwMode == kModeCommand) {
                HBRUSH bg = CreateSolidBrush(RGB(64, 64, 64));
                RECT r;
                GetClientRect(hwnd, &r);

                FillRect(hdc, &r, bg);
                DeleteObject(bg);
            } else {
                DWORD ret = 0;

                if ((ret = ti->hEmulator->paint(hwnd,
                        (LPVOID)ti->hEmulator->emulator_data, hdc, TRUE)) != 0) {
                    /* An error occurred while painting the text */
                    DWORD dwError = GetLastError();
                    ReportError(dwError);
                }
            }

            EndPaint (hwnd, &ps);
        }
        return 0;
    case WM_CHAR:
        {
            if (ti->dwMode == kModeConnect) {
                if (SendData(&ti->hCommDev, (LPVOID)&wParam, _tcsclen((LPCTSTR)&wParam)) != 0) {
                    DWORD dwError = GetLastError();
                    ReportError(dwError);
                }
            }
        }
        return 0;
    case WM_KEYUP:
        {
            if (ti->dwMode == kModeConnect) {
                LPCSTR data = ti->hEmulator->escape_input((LPVOID)ti->hEmulator->emulator_data, wParam);
                if (data == NULL)
                    return 0;

                if (SendData(&ti->hCommDev, (LPVOID)data, strlen(data)) != 0) {
                    DWORD dwError = GetLastError();
                    ReportError(dwError);
                }
            }
        }
        return 0;
    case TWM_RXDATA:
        {
            if (ti->dwMode == kModeConnect) {
                DWORD ret = 0;

                ti->hEmulator->receive(ti->hEmulator->emulator_data, (BYTE*)wParam, lParam);
                free((BYTE*)wParam);
                
                if ((ret = ti->hEmulator->paint(hwnd,
                        (LPVOID)ti->hEmulator->emulator_data, NULL, FALSE)) != 0) {
                    /* An error occurred while painting the text */
                    DWORD dwError = GetLastError();
                    ReportError(dwError);
                }
            }
        }
        return 0;
    case TWM_TXDATA:
        {
            if (ti->dwMode == kModeConnect) {
                BYTE* data = (BYTE*)wParam;
                if (data == NULL)
                    return 0;

                if (SendData(&ti->hCommDev, (LPVOID)data, lParam) != 0) {
                    DWORD dwError = GetLastError();
                    ReportError(dwError);
                }

                free(data);
            }
        }
        return 0;
    case WM_DESTROY:
        {
            CommandMode(hwnd);
            if (ti->hReadLoop != NULL) {
                CloseHandle(ti->hReadLoop);
            }
            PostQuitMessage(0);
        }
        return 0;
    }
    return DefWindowProc (hwnd, message, wParam, lParam);
}

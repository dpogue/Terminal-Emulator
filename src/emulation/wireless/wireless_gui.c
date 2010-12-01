#include <WindowsX.h>
#include "wireless.h"
#include "resource.h"

typedef struct _gui_hack {
    WirelessData* dat;
    WNDPROC oldProc;
} GUIHack;

BOOL CALLBACK WirelessDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        {
            WirelessData* dat = (WirelessData*)lParam;
            WNDPROC oldProc = SubclassWindow(GetDlgItem(hDlg, FILE_PATH), DragDropProc);
            GUIHack* g = (GUIHack*)malloc(sizeof(GUIHack));
            g->dat = dat;
            g->oldProc = oldProc;

            SetWindowLongPtr(hDlg, GWL_USERDATA, (LONG_PTR)g);
        }
        return TRUE;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
    		case SELECT_FILE:
                {
                    OPENFILENAME ofn;
                    TCHAR szFilename[MAX_PATH] = TEXT("");
                    TCHAR szFilepath[MAX_PATH] = TEXT("");
                    TCHAR szFilter[] = TEXT ("Text Files (*.TXT)\0*.txt\0")  \
                                       TEXT ("ASCII Files (*.ASC)\0*.asc\0") \
                                       TEXT ("All Files (*.*)\0*.*\0\0");

                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.hwndOwner = GetParent(hDlg);
                    ofn.hInstance = GetWindowInstance(GetParent(hDlg));
                    ofn.lpstrFile = szFilepath;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrFileTitle = szFilename;
                    ofn.nMaxFileTitle = MAX_PATH;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    ofn.nFileOffset = 0;
                    ofn.nFileExtension = 0;
                    ofn.lpstrInitialDir = NULL;
                    ofn.lpstrTitle = NULL;
                    ofn.nMaxCustFilter = 0;
                    ofn.nFilterIndex = 1;
                    ofn.lCustData = 0;
                    ofn.lpfnHook = NULL;
                    ofn.lpTemplateName = NULL;
                    ofn.lpstrFilter = szFilter;
                    ofn.lpstrCustomFilter = NULL;
                    ofn.lpstrDefExt = TEXT("txt");

                    if(GetOpenFileName(&ofn))
                        SetDlgItemText(hDlg, FILE_PATH, ofn.lpstrFile);
                }
                return TRUE;
            case SEND_FILE:
                {
                    GUIHack* g = (GUIHack*)GetWindowLongPtr(hDlg, GWL_USERDATA);
                    WirelessData* dat = g->dat;
                    TCHAR filename[MAX_PATH];
                    
                    GetDlgItemText(hDlg, FILE_PATH, filename, MAX_PATH);
                    if (filename != NULL) {
                        dat->send.fd = _tfopen(filename, TEXT("rb"));
                        SetClassLong(hDlg, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_WAIT));
                    }
                }
                return TRUE;
            default:
                return FALSE;
            }
        }
    case WM_CLOSE:
        ShowWindow(hDlg, SW_HIDE);
    }
    return FALSE;

#if 0
	static HWND parent;
	static HMENU hMenu;
	RECT rc, rcDlg, rcOwner; 
	int id = GetWindowLong (hwnd, GWL_ID);

	switch(msg){
	case WM_COMMAND:
		switch ( LOWORD( wParam ) ) {
		case SELECT_FILE:
		case SEND_FILE:
			/* add send file */
			break;
		case WM_DESTROY://how to handle a destroy (close window app) msg
			PostQuitMessage(0);
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		default:
			return FALSE;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	} 
	return DefWindowProc(hwnd, msg, wParam, lParam);
#endif
}

LRESULT CALLBACK DragDropProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND parent = GetParent(hwnd);
    GUIHack* g = (GUIHack*)GetWindowLongPtr(parent, GWL_USERDATA);
    WNDPROC old = g->oldProc;

    switch (message) {
    case WM_DROPFILES:
        {
			HDROP hDrop = (HDROP)wParam;
			TCHAR buffer[MAX_PATH];
			int nFilesDropped = DragQueryFile (hDrop, 0, buffer, MAX_PATH);

			if (nFilesDropped) {
                SetWindowText(hwnd, buffer);
			}

			DragFinish (hDrop);
        }
        return 0;
    }

    return CallWindowProc(old, hwnd, message, wParam, lParam);
}

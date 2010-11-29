#ifndef _PREFSDLG_H_
#define _PREFSDLG_H_

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>
#include "pluginloader.h"

/* Dialog constants */
#define PREFERENCES_DLG 300
#define PREFERENCES_PLUGINS 301
#define PREFERENCES_SAVE 302

#define PREFERENCES_NONE 310
#define PREFERENCES_ONE 311
#define PREFERENCES_TWO 312

#define PREFS_CHECK_OFFSET 3000
#define PREFS_RADIO_OFFSET 3100

typedef struct _plugin_state {
    DWORD dwIndex;
    LPCTSTR lpszName;
    BOOLEAN bEnabled;
    BOOLEAN bActive;
} PluginState;

/**
 * Creates the preferences modeless dialog window.
 * @implementation prefsdlg.c
 */
HWND CreatePreferencesDlg(HWND parent);

/**
 * Shows the preferences dialog window.
 * @implementation prefsdlg.c
 */
VOID ShowPreferencesDlg(HWND parent);

/**
 * Message handling for Windows messages.
 * @implementation prefsdlg.c
 */
LRESULT CALLBACK PreferencesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

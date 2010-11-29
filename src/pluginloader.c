/**
 * @filename pluginloader.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the implementations of functions related
 * to the loading of emulation plugins.
 */
#include "pluginloader.h"
#include "terminal.h"

/**
 * Iterator over the available plugins, calling a specified function for each.
 *
 * @param HWND hwnd         The handle to the application window.
 * @param PluginFunctor fn  The function to be called for each plugin.
 * @param LPVOID result     Data to be manipulated by the function.
 * @return none
 */
VOID ForeachPlugin(HWND hwnd, PluginFunctor fn, LPVOID result) {
    WIN32_FIND_DATA ffd;
    TCHAR szAppPath[MAX_PATH];
    TCHAR szDir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;

    /* Get the application directory then search a subdirectory of that for DLL files */
    GetModuleFileName(0, szAppPath, sizeof(szAppPath) - 1);
    StringCchCopy(szDir, _tcsrchr(szAppPath, '\\') - szAppPath + 1, szAppPath);
    StringCchCat(szDir, MAX_PATH, TEXT("\\emulation\\*.dll"));

    hFind = FindFirstFile(szDir, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        DWORD dwError = GetLastError();
        ReportError(dwError);
        return;
    }

    /* Loop over the plugin files */
    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            /* Call the functor with the file information */
            fn(hwnd, ffd, result);
        }
    }
    while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
}

/**
 * Functor to increment a count for each plugin that is found.
 *
 * @param HWND hwnd             The handle of the application window.
 * @param WIN32_FIND_DATA ffd   The plugin file data.
 * @param LPVOID result         A pointer to an int to be incremented.
 * @return none
 */
VOID CountPluginsFunctor(HWND hwnd, WIN32_FIND_DATA ffd, LPVOID result) {
    INT* count = (INT*)result;
    (*count)++;
}

/**
 * Count the number of plugins available.
 *
 * @param HWND hwnd    The handle to the application window.
 * @return The number of plugins found.
 */
INT CountPlugins(HWND hwnd) {
    INT count = 0;

    /* Iterate over the plugins, incrementing the count */
    ForeachPlugin(hwnd, CountPluginsFunctor, (LPVOID)&count);

    return count;
}

/**
 * Functor to load each plugin as it is found.
 *
 * @param HWND hwnd             The handle of the application window.
 * @param WIN32_FIND_DATA ffd   The plugin file data.
 * @param LPVOID result         A pointer to a TerminalModes structure.
 * @return none
 */
VOID LoadPluginsFunctor(HWND hwnd, WIN32_FIND_DATA ffd, LPVOID result) {
    typedef BOOLEAN (*init_plugin)(HWND hwnd, Emulator** e);

    TCHAR szAppPath[MAX_PATH];
    TCHAR szPlugin[MAX_PATH];
    HMODULE lib;
    DWORD i;
    TerminalModes* modes = (TerminalModes*)result;

    /* Find the first blank space to store this plugin */
    for (i = 1; i < modes->dwCount; i++) {
        if (modes->lpEmulators[i] == NULL)
            break;
    }

    /* Protect against memory corruption */
    if (i >= modes->dwCount)
        return;

    /* Get the application directory in order to get the full path */
    GetModuleFileName(0, szAppPath, sizeof(szAppPath) - 1);
    StringCchCopy(szPlugin, _tcsrchr(szAppPath, '\\') - szAppPath + 1, szAppPath);
    StringCchCat(szPlugin, MAX_PATH, TEXT("\\emulation\\"));
    StringCchCat(szPlugin, MAX_PATH, ffd.cFileName);

    /* Try to load a handle to the plugin module */
    if ((lib = LoadLibrary(szPlugin)) != 0) {
        /* Get the address of the plugin's init function */
        init_plugin ip = (init_plugin)GetProcAddress(lib, "emulator_init_plugin");
        if (ip != NULL) {
            /* Initialise the plugin */
            ip(hwnd, &(modes->lpEmulators[i]));
        }
    }
}

/**
 * Load all of the emulation plugins.
 *
 * @param HWND hwnd     The handle to the application window.
 * @return none
 */
VOID LoadPlugins(HWND hwnd) {
    TerminalModes* modes = (TerminalModes*)malloc(sizeof(TerminalModes));
    INT count = CountPlugins(hwnd);
    INT i = 0;

    /* The total number of emulation modes is the number of plugins + 1 (no emulation) */
    modes->dwCount = ++count;

    /* Allocate pointer space for each mode, and set them to NULL initially */
    modes->lpEmulators = (Emulator**)malloc(sizeof(Emulator*) * count);
    for (i = 0; i < count; i++) {
        modes->lpEmulators[i] = NULL;
    }

    /* The first mode is always No Emulation */
    modes->lpEmulators[0] = none_init(hwnd);

    /* Load the rest of the emulation mode plugins */
    ForeachPlugin(hwnd, LoadPluginsFunctor, (LPVOID)modes);

    /* Store it in the class data */
    SetClassLongPtr(hwnd, 0, (LONG_PTR)modes);
}

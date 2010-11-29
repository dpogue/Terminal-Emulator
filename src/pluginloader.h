/**
 * @filename pluginloader.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 28
 * @project Terminal Emulator
 *
 * This file contains the definitions and prototypes for functions
 * related to the loading of emulation plugins.
 */
#ifndef _PLUGINLOADER_H_
#define _PLUGINLOADER_H_

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <strsafe.h>

typedef VOID (*PluginFunctor)(HWND, WIN32_FIND_DATA, LPVOID);

/**
 * Iterator over the available plugins, calling a specified function for each.
 * @implementation pluginloader.c
 */
VOID ForeachPlugin(HWND hwnd, PluginFunctor fn, LPVOID result);

/**
 * Functor to increment a count for each plugin that is found.
 * @implementation pluginloader.c
 */
VOID CountPluginsFunctor(HWND hwnd, WIN32_FIND_DATA ffd, LPVOID result);

/**
 * Count the number of plugins available.
 * @implementation pluginloader.c
 */
INT CountPlugins(HWND hwnd);

/**
 * Functor to load each plugin as it is found.
 * @implementation pluginloader.c
 */
VOID LoadPluginsFunctor(HWND hwnd, WIN32_FIND_DATA ffd, LPVOID result);

/**
 * Load all of the emulation plugins.
 * @implementation pluginloader.c
 */
VOID LoadPlugins(HWND hwnd);

#endif

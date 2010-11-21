/**
 * @filename emulation.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 10
 * @project Terminal Emulator
 *
 * This file contains the definition of the emulation plugin interface.
 */
#ifndef _EMULATION_H_
#define _EMULATION_H_

#include <Windows.h>
#include <tchar.h>

typedef struct _emulator {
    DWORD dwVersion;

    /* @since 1 */
    LPVOID emulator_data;

    /* @since 1 */
    LPCTSTR (*emulation_name)(void);

    /* @since 1 */
    LPCSTR (*escape_input)(LPVOID data, DWORD input);

    /* @since 1 */
    DWORD (*receive)(LPVOID data, BYTE* rx, DWORD len);

    /* @since 2 */
    DWORD (*paint)(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force);

    /* @since 2 */
    DWORD (*on_connect)(LPVOID data);

    /* @since 3 */
    DWORD (*on_disconnect)(LPVOID data);

    /* @since 3 */
    BOOLEAN (*wnd_proc_override)(LPVOID data, LPMSG msg);

    /* @since 3 */
    HMENU (*emulator_menu)(void);
} Emulator;

#ifdef __cplusplus
#define DllExport extern "C" __declspec(dllexport)
#else
#define DllExport __declspec(dllexport)
#endif


#define EMULATOR_HAS_FUNC(emu, func) \
    ((emu != NULL) && (emu->func != NULL))

#define EMULATOR_INIT_PLUGIN(initfunc) \
    DllExport BOOLEAN emulator_init_plugin(HWND hwnd, Emulator** e) { \
        *e = initfunc(hwnd); \
        if (*e == NULL) return FALSE; \
        if ((*e)->dwVersion <= 0) return FALSE; \
        if ((*e)->emulation_name == NULL) return FALSE; \
        if ((*e)->receive == NULL) return FALSE; \
        if ((*e)->paint == NULL) return FALSE; \
        return TRUE; \
    }

#endif

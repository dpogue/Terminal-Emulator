/**
 * @filename emulation.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 10 20
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the definition of the emulation plugin interface.
 */
#ifndef _EMULATION_H_
#define _EMULATION_H_

#include <Windows.h>
#include <tchar.h>

#define EMULATOR_HAS_FUNC(emu, func) \
    ((emu != NULL) && (emu->func != NULL))

typedef struct _emulator {
    DWORD dwVersion;

    /* @since 2 */
    void* emulator_data;

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
    BOOLEAN (*wnd_proc_override)(LPMSG msg);

    /* @since 3 */
    HMENU (*emulator_menu)(void);
} Emulator;

#endif

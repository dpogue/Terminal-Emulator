/**
 * @filename emulation.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 10 20
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the definition of the emulation plugin interface.
 */
#include <Windows.h>
#include <tchar.h>

typedef struct _emulator {
    DWORD dwVersion;

    /* @since 2 */
    void* emulator_data;

    /* @since 1 */
    LPCTSTR (*emulation_name)(void);

    /* @since 1 */
    LPCSTR (*escape_input)(LPVOID data, DWORD input);

    /* @since 1 */
    DWORD (*receive)(LPVOID data, LPCTSTR rx);

    /* @since 2 */
    DWORD (*paint)(HWND hwnd, LPVOID data, HDC hdc, BOOL force);

    /* @since 2 */
    DWORD (*on_connect)(LPVOID data);
} Emulator;

#ifndef _RFID_H_
#define _RFID_H_

#include <Windows.h>
#include <tchar.h>
#include "../../defines.h"
#include "../../emulation.h"
#include "resource.h"

#include "rfid_structures.h"

/* Prefix these with RFID because some conflict with Windows constants! */
enum RFID_StatusCodes {
	RFIDERROR_NONE = 0x00,
	RFIDERROR_TOKEN_NOT_PRESENT = 0x01,
	RFIDERROR_INVALID_RF_FORMAT = 0x05,
	RFIDERROR_DEVICE_ID_INVALID = 0x0E,
	RFIDERROR_ILLEGAL_ACTION = 0x10,
	RFIDERROR_WRONG_DOWNLOAD_STATE = 0x11,
	RFIDERROR_WRITE_FAILED = 0x12,
	RFIDERROR_INVALID_ADDRESS = 0x13
};

typedef struct _rfid_data {
    HWND console;
    HWND dialog;
    TCHAR screen[24][81];
	BYTE screenrow;
} RFID_Data;

LPCTSTR rfid_entity_name(BYTE entity);

/**
 * @implementation rfid_dlg.c
 */
BOOL CALLBACK rfid_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
        LPARAM lParam);

#endif

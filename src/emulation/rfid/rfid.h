/**
 * @filename rfid.h
 * @author Darryl Pogue & Joel Stewart
 * @designer Darryl Pogue, Terence Stenvold, Joel Stewart
 * @date 2010 11 10
 * @project Terminal Emulator: RFID Plugin
 *
 * This file contains all of the function prototypes and enum declarations
 * for the RFID plugin.
 */
#ifndef _RFID_H_
#define _RFID_H_

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
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

enum RFID_Baud {
    RFID_BAUD_9600    = 0,
    RFID_BAUD_19200   = 1,
    RFID_BAUD_57600   = 2,
    RFID_BAUD_115200  = 3,
    RFID_BAUD_38400   = 4
};

typedef struct _rfid_data {
    HWND console;
    HWND dialog;
    TCHAR screen[24][81];
    BYTE screenrow;
} RFID_Data;

/**
 * @implementation rfid_util.c
 */
LPCTSTR rfid_entity_name(BYTE entity);

/**
 * @implementation rfid_util.c
 */
RFID_BCC rfid_calc_bcc(LPVOID message, WORD size);

/**
 * @implementation rfid_util.c
 */
void rfid_getversion_request(RFID_A2D_GetVersion** msg);

/**
 * @implementation rfid_util.c
 */
void rfid_findtoken_request(RFID_A2D_FindToken** msg);

/**
 * @implementation rfid_util.c
 */
void rfid_setdriver_request(RFID_A2D_SetDriver** msg, BYTE drivers);

/**
 * @implementation rfid_util.c
 */
void rfid_setbaud_request(RFID_A2D_SetBaud** msg, BYTE baud);

/**
 * @implementation rfid_util.c
 */
void rfid_transon_request(RFID_A2D_TransOn** msg, BYTE entity);

/**
 * @implementation rfid_util.c
 */
void rfid_transoff_request(RFID_A2D_TransOff** msg, BYTE entity);

/**
 * @implementation rfid_dlg.c
 */
BOOL CALLBACK rfid_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
        LPARAM lParam);

#endif

#ifndef _RFID_H_
#define _RFID_H_

#include <Windows.h>
#include <tchar.h>
#include "../../defines.h"
#include "../../emulation.h"

typedef struct _rfid_data {
    HWND hwnd;
    TCHAR screen[24][81];
} RFID_Data;

#endif
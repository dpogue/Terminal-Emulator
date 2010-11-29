/**
 * @filename emulation_none.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 11
 * @project Terminal Emulator
 *
 * This file contains the prototypes for the barebones emulation mode.
 */

#ifndef _EMULATION_NONE_H_
#define _EMULATION_NONE_H_

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "defines.h"
#include "emulation.h"

typedef struct _none_data {
    TCHAR screen[24][81];
    BYTE screenrow;
    BYTE screencol;
} NoneData;

/**
 * Initialise the default barebones emulator.
 * @implementation emulation_none.c
 */
Emulator* none_init(HWND hwnd);

#endif

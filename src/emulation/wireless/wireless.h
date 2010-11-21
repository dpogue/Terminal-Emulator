/**
 * @filename wireless.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 21
 * @project Terminal Emulator
 *
 * This file contains the prototypes for the wireless protocol design.
 */

#ifndef _WIRELESS_H_
#define _WIRELESS_H_

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "../../emulation.h"
#include "wireless_frame.h"

enum WirelessState {
    kIdleState,
    kSentENQState,
    kSendingState,
    kWaitFrameACKState,
    kGotENQState,
    kReadFrameState,
    kSendFrameACKState,
    kGotRVIState
};

typedef struct _wireless_data {
    TCHAR screen[24][81];
    BYTE screenrow;

    WORD state;
    FILE* fdSend;
    FILE* fdRecv;
} WirelessData;

/**
 * Initialise the default barebones emulator.
 * @implementation emulation_none.c
 */
Emulator* wireless_init(HWND hwnd);

#endif

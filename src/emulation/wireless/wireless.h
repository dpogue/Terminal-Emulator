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
#include <stdlib.h>
#include "../../defines.h"
#include "../../emulation.h"
#include "wireless_structures.h"
#include "wireless_frame.h"

#define RVI 0x11
#define ACK 0x06
#define ENQ 0x05
#define EOT 0x04
#define SOF 0x01

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


void SendByte(HWND hwnd, BYTE value);

VOID CALLBACK SentENQTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time);
VOID CALLBACK WaitFrameACKTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time);
VOID CALLBACK RandDelayTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time);

#endif

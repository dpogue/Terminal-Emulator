/**
 * @filename wireless_structures.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 22
 * @project Terminal Emulator
 *
 * This file contains the prototypes and structures for data frames for the
 * wireless protocol design.
 */
#ifndef _WIRELESS_STRUCTURES_H_
#define _WIRELESS_STRUCTURES_H_

#include <Windows.h>

#pragma pack(push)
#pragma pack(1)
typedef struct _wireless_frame {
    BYTE start;
    BYTE sequence;
    WORD size;
    BYTE data[1019];
    BYTE crc;
} WirelessFrame;
#pragma pack(pop)

typedef struct _wireless_comm {
    FILE* fd;
    WirelessFrame* frame;
    BYTE sequence;
} WirelessComm;

enum Timers {
    kSentENQTimer = 1,
    kWaitFrameACKTimer,
    kReadFrameTimer,
    kRandDelayTimer,
    kMaxTimer = kRandDelayTimer
};

#define TO_ENQ 1000
#define TO_FRAME 3000
#define TO_READ 1500
#define TO_RAND_MIN 0
#define TO_RAND_MAX 1000

typedef struct _wireless_data {
    TCHAR screen[24][81];
    BYTE screenrow;
    HWND hwnd;
    WORD state;
    UINT_PTR timeout;
    WirelessComm send;
    WirelessComm read;

    BOOLEAN midFrame;
    WORD readPos;
    BYTE counters[kMaxTimer];
    HWND hDlg;
    INT nAcks;
    INT nErrors;
    INT nPackets;
} WirelessData;

#endif
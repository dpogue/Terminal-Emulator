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

typedef struct _wireless_data {
    TCHAR screen[24][81];
    BYTE screenrow;
    HWND hwnd;
    WORD state;
    WirelessComm send;
    WirelessComm read;

    BOOLEAN midFrame;
    WORD readPos;
} WirelessData;

#endif
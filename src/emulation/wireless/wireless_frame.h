/**
 * @filename wireless_frame.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 21
 * @project Terminal Emulator
 *
 * This file contains the prototypes and structures for data frames for the
 * wireless protocol design.
 */

#ifndef _WIRELESS_FRAME_H_
#define _WIRELESS_FRAME_H_

typedef struct _wireless_frame {
    BYTE start;
    BYTE sequence;
    WORD size;
    BYTE data[1019];
    BYTE crc;
} WirelessFrame;

#endif
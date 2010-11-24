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

#include "wireless.h"
#include "wireless_structures.h"
#include "crc.h"

#define READ_SIZE 1019
#define CRC_SIZE 1023

WORD read_file(WirelessData *data, WirelessFrame *wf);
crc crc_calculate(WirelessFrame *wf);
WirelessFrame* build_frame(WirelessData *data);

BOOLEAN verify_frame(WirelessFrame* frm);

#endif
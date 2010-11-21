/**
 * @filename rfid_util.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 11 10
 * @project Terminal Emulator: RFID Plugin
 *
 * This file contains the implementations for utility functions used by the
 * RFID plugin.
 */
#include "rfid.h"

LPCTSTR rfid_entity_name(BYTE entity) {
    switch(entity) {
    case 0x01:
        return TEXT("Application Layer");
    case 0x02:
        return TEXT("ISO 14443-A");
    case 0x03:
        return TEXT("ISO 14443-B");
    case 0x04:
        return TEXT("ISO 15693");
    case 0x05:
        return TEXT("Tag-It");
    case 0x06:
        return TEXT("LF DST");
    case 0x07:
        return TEXT("ISO 14443-4");
    case 0x08:
        return TEXT("TI Apollo");
    case 0x11:
        return TEXT("Boot Loader");
    default:
        return TEXT("Unknown");
    }
}

RFID_BCC rfid_calc_bcc(LPVOID message, WORD size) {
    BYTE lrc = 0;
    DWORD i = 0;
    RFID_BCC bcc;

    for (i = 0; i < size; i++) {
        lrc ^= *((BYTE*)message + i);
    }
    
    bcc.lrc = lrc;
    bcc.i_lrc = (lrc ^ 0xFF);

    return bcc;
}

void rfid_getversion_request(RFID_A2D_GetVersion** msg) {
    *msg = (RFID_A2D_GetVersion*)malloc(sizeof(RFID_A2D_GetVersion));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_GetVersion);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = 0x1;
    (*msg)->header.command2 = 0x40;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

void rfid_findtoken_request(RFID_A2D_FindToken** msg) {
    *msg = (RFID_A2D_FindToken*)malloc(sizeof(RFID_A2D_FindToken));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_FindToken);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = 0x1;
    (*msg)->header.command2 = 0x41;
    (*msg)->timeout = 0xA;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

void rfid_setdriver_request(RFID_A2D_SetDriver** msg, BYTE drivers) {
    *msg = (RFID_A2D_SetDriver*)malloc(sizeof(RFID_A2D_SetDriver));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_SetDriver);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = 0x1;
    (*msg)->header.command2 = 0x43;
    (*msg)->drivers = drivers;
    (*msg)->active = 0x3;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

void rfid_setbaud_request(RFID_A2D_SetBaud** msg, BYTE baud) {
    *msg = (RFID_A2D_SetBaud*)malloc(sizeof(RFID_A2D_SetBaud));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_SetBaud);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = 0x1;
    (*msg)->header.command2 = 0x46;
    (*msg)->baud = baud;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

void rfid_transon_request(RFID_A2D_TransOn** msg, BYTE entity) {
    *msg = (RFID_A2D_TransOn*)malloc(sizeof(RFID_A2D_TransOn));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_TransOn);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = entity;
    (*msg)->header.command2 = 0x48;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

void rfid_transoff_request(RFID_A2D_TransOff** msg, BYTE entity) {
    *msg = (RFID_A2D_TransOff*)malloc(sizeof(RFID_A2D_TransOff));
    (*msg)->header.soframe = 0x1;
    (*msg)->header.length = sizeof(RFID_A2D_TransOff);
    (*msg)->header.deviceID = 0x3;
    (*msg)->header.command1 = entity;
    (*msg)->header.command2 = 0x49;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}
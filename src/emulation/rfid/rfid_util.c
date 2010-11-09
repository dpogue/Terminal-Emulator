#include "rfid.h"

LPCTSTR rfid_entity_name(BYTE entity) {
	switch(entity) {
	case 0x01:
		return TEXT("Application Layer");
	case 0x02:
		return TEXT("14443-A");
	case 0x03:
		return TEXT("14443-B");
	case 0x04:
		return TEXT("15693");
	case 0x05:
		return TEXT("Tag-It");
	case 0x06:
		return TEXT("LF DST");
	case 0x07:
		return TEXT("14443-4");
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
    (*msg)->header.command2 = 0x40;
	(*msg)->timeout = 0xA;
    (*msg)->bcc = rfid_calc_bcc((LPVOID)*msg, (*msg)->header.length - 2);
}

#ifndef _RFID_STRUCTURES_H_
#define _RFID_STRUCTURES_H_

#pragma pack(push)
#pragma pack(1)

typedef struct _rfid_header {
    BYTE soframe;
	WORD length;
	BYTE deviceID;
	BYTE command1;
	BYTE command2;
} RFID_Header;

typedef struct _rfid_bcc {
	BYTE lrc;
	BYTE i_lrc;
} RFID_BCC;

typedef struct _rfid_a2d_getversion {
    RFID_Header header;
    RFID_BCC bcc;
} RFID_A2D_GetVersion;

typedef struct _rfid_d2a_getversion {
	RFID_Header header;
	BYTE status;
	BYTE entityID;
} RFID_D2A_GetVersion;

typedef struct _rfid_a2d_findtoken {
    RFID_Header header;
    BYTE timeout;
    RFID_BCC bcc;
} RFID_A2D_FindToken;

typedef struct _rfid_d2a_findtoken {
	RFID_Header header;
	BYTE status;
	BYTE entityID;
} RFID_D2A_FindToken;

typedef struct rfid_a2d_setdriver {
	RFID_Header header;
	BYTE drivers;
	BYTE active;
	RFID_BCC bcc;
} RFID_A2D_SetDriver;

typedef struct _rfid_a2d_setbaud {
	RFID_Header header;
	BYTE baud;
	RFID_BCC bcc;
} RFID_A2D_SetBaud;

typedef struct _rfid_d2a_setbaud {
	RFID_Header header;
	BYTE status;
	RFID_BCC bcc;
} RFID_D2A_SetBaud;

#pragma pack(pop)

#endif

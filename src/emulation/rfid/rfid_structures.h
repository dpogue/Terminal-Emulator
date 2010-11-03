#ifndef _RFID_STRUCTURES_H_
#define _RFID_STRUCTURES_H_

#pragma pack(push)
#pragma pack(1)

typedef struct _rfid_header {
	WORD length;
	BYTE deviceID;
	BYTE command1;
	BYTE command2;
} RFID_Header;

typedef struct _rfid_bcc {
	BYTE lrc;
	BYTE i_lrc;
} RFID_BCC;

typedef struct _rfid_d2a_getversion {
	RFID_Header header;
	BYTE status;
	BYTE entityID;
} RFID_D2A_GetVersion;

typedef struct _rfid_d2a_findtoken {
	RFID_Header header;
	BYTE status;
	BYTE entityID;
} RFID_D2A_FindToken;

#pragma pack(pop)

#endif
#include "rfid.h"
#include "../../terminal.h"

/**
 * Provides a name for the emulation mode.
 *
 * @returns A user-friendly name for this emulation.
 */
LPCTSTR rfid_emulation_name(void) {
    return TEXT("RFID");
}

/**
 * Escapes keyboard input for this emulation mode before passing it to the
 * transmission layer.
 *
 * @param LPVOID data   The emulator data field
 * @param DWORD input   The keyboard input to be escaped.
 *                      This stores the virtual key code that was pressed.
 *
 * @returns NULL if no escaping is needed, otherwise the escaped sequence.
 */
LPCSTR rfid_escape_input(LPVOID data, DWORD input) {
	return NULL;
}

/**
 * Parses received data and handles any escape sequences, control characters
 * or terminal commands.
 *
 * @param LPVOID data   The emulation mode data
 * @param LPCTSTR rc    The received data as a string.
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD rfid_receive(LPVOID data, BYTE* rx, DWORD len) {
    static BYTE* buffer = NULL;
    static BYTE bcc = 0;
    static WORD buflen = 0;
    static WORD length = 0;

    if (buffer != NULL) {
        DWORD i = 0;
        for (i = 0; i < len; i++) {
            bcc ^= rx[i];
            buffer[buflen] = rx[i];
            buflen++;
        }
    } else {
        DWORD i = 0;

        if (rx[0] != 0x01) {
            /* Expected start of frame and got... something else */
            return 1;
        }

        length = (WORD)rx[1];

        buffer = (BYTE*)calloc(1, length);

        for (i = 0; i < len; i++) {
            bcc ^= rx[i];
            buffer[i] = rx[i];
            buflen++;
        }
    }

    if (buflen == length) { /* We have the entire message */
		DWORD i = 0;
		LPTSTR prt;

        /* Check the BCC bytes to make sure it's correct */
        if (buffer[buflen - 1] != bcc && buffer[buflen] != (bcc ^ 0xFF)) {
            free(buffer);
            buffer = NULL;
            buflen = 0;
            length = 0;
            bcc = 0;
            return 2;
        }

        /* Parse! */
		prt = (LPTSTR)malloc(sizeof(TCHAR)*(length * 3));
		prt[0] = 0;
		for (i = 0; i < length; i++) {
			_stprintf(prt, TEXT("%s %02X"), prt, buffer[i]);
		}
		MessageBox(NULL, prt, NULL, MB_ICONWARNING);

        free(buffer);
        buffer = NULL;
        buflen = 0;
        length = 0;
        bcc = 0;
    }

	return 0;
}

/**
 * Paint the screen according to the rules of this emulation mode.
 *
 * @param HWND hwnd     Handle to the application window.
 * @param LPVOID data   The emulation mode data
 * @param HDC hdc       The handle to the device context.
 *                      If this is NULL, GetDC will be called.
 * @param BOOLEAN force Force a repaint of the whole screen if true.
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD rfid_paint(HWND hwnd, LPVOID data, HDC hdc, BOOLEAN force) {
	return 0;
}

/**
 * Performs any actions that are necessary immediately after connecting with
 * this emulation mode.
 *
 * @param LPVOID data   The emulation mode data
 *
 * @returns int 0 on success, greater than 0 otherwise.
 */
DWORD rfid_on_connect(LPVOID data) {
    RFID_Data* dat = (RFID_Data*)data;

	CHAR* msg = "\x01\x08\x00\x03\x01\x40\x4B\xB4\0";
    SendMessage(dat->hwnd, TWM_TXDATA, (WPARAM)msg, 8);
    return 0;
}

Emulator emu_rfid =
{
    2,                      /** << Emulator structure version */
    NULL,                   /** << Emulator data pointer */
    &rfid_emulation_name,  /** << Function returning emulator name */
    &rfid_escape_input,    /** << Function to escape keyboard input */
    &rfid_receive,         /** << Function to handled received data */
    &rfid_paint,           /** << Function to repaint the screen */
    &rfid_on_connect       /** << Function to call upon connection */
};

Emulator* rfid_init(HWND hwnd) {
	Emulator* e = &emu_rfid;
    DWORD y = 0;
    DWORD x = 0;
    RFID_Data* data = (RFID_Data*)malloc(sizeof(RFID_Data));

    data->hwnd = hwnd;

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            data->screen[y][x] = (x == 80) ? '\0' : ' ';
        }
    }

	e->emulator_data = data;

	return e;
}
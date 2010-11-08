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

	RFID_Data* dat = (RFID_Data*)data;

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
		RFID_Header head;

        /* Check the BCC bytes to make sure it's correct */
        if (buffer[buflen - 1] != bcc && buffer[buflen] != (bcc ^ 0xFF)) {
            free(buffer);
            buffer = NULL;
            buflen = 0;
            length = 0;
            bcc = 0;
            return 2;
        }

		/*head = *((RFID_Header*)buffer);*/

		head.length = (WORD)*(buffer + 1);
		head.deviceID = (BYTE)*(buffer + 3);
		head.command1 = (BYTE)*(buffer + 4);
		head.command2 = (BYTE)*(buffer + 5);

		switch (head.command2) {
		case 0x40:
			{
				RFID_D2A_GetVersion msg;
				WORD numMessages = (head.length - sizeof(RFID_Header) - 2 - 2) / 3;
				WORD pos = 7;

				msg.header = head;
				msg.status = (BYTE)*(buffer + 6);

				while (numMessages-- > 0) {
					BYTE entity = (BYTE)*(buffer + pos);
					WORD version = (WORD)(*(buffer + (pos + 1)) << 8 | *(buffer + (pos + 2)));

					prt = (LPTSTR)malloc(sizeof(TCHAR)*80);
					prt[0] = 0;
					_stprintf(prt, TEXT("    version %d.%d.%d (%s Module)"), (version & 0xF00) >> 8,
						(version & 0xF0) >> 4, (version & 0xF), rfid_entity_name(entity));

					_tcsncpy(dat->screen[dat->screenrow++], prt, 80);
					pos += 3;
				}
			}
			break;
		case 0x41:
			{
			}
			break;
		}

        /* Parse! */
		prt = (LPTSTR)malloc(sizeof(TCHAR)*(length * 3));
		prt[0] = 0;
		for (i = 0; i < length; i++) {
			_stprintf(prt, TEXT("%s %02X"), prt, buffer[i]);
		}
		/*MessageBox(NULL, prt, NULL, MB_ICONWARNING);*/
        SetDlgItemText(dat->dialog, IDC_EDIT1, prt);

        free(buffer);
        buffer = NULL;
        buflen = 0;
        length = 0;
        bcc = 0;

		InvalidateRect(dat->hwnd, NULL, TRUE);
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
	RFID_Data* dat = (RFID_Data*)data;
	TEXTMETRIC tm;
	BYTE y = 0;
	BOOLEAN bGotDC = FALSE;

	if (hdc == NULL) {
        hdc = GetDC(hwnd);
        bGotDC = TRUE;
    }

	SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
    GetTextMetrics(hdc, &tm);

	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));

	for (y = 0; y < 24; y++) {
		TextOut(hdc, 0, y * (tm.tmExternalLeading + tm.tmHeight), dat->screen[y], _tcslen(dat->screen[y]));
	}

	if (bGotDC) {
        ReleaseDC(hwnd, hdc);
    }

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

    SetDlgItemText(data->dialog, IDC_EDIT1, TEXT("CONNECTED"));

    ShowWindow(data->dialog, SW_SHOW);
    ShowWindow(data->console, SW_HIDE);


	CHAR* msg = "\x01\x08\x00\x03\x01\x40\x4B\xB4\0";
    SendMessage(dat->hwnd, TWM_TXDATA, (WPARAM)msg, 8);
    return 0;
}

Emulator emu_rfid =
{
    3,                      /** << Emulator structure version */
    NULL,                   /** << Emulator data pointer */
    &rfid_emulation_name,   /** << Function returning emulator name */
    &rfid_escape_input,     /** << Function to escape keyboard input */
    &rfid_receive,          /** << Function to handled received data */
    &rfid_paint,            /** << Function to repaint the screen */
    &rfid_on_connect,       /** << Function to call upon connection */
    NULL,
    NULL,
    NULL
};

Emulator* rfid_init(HWND hwnd) {
	Emulator* e = &emu_rfid;
    DWORD y = 0;
    DWORD x = 0;
    RFID_Data* data = (RFID_Data*)malloc(sizeof(RFID_Data));
    HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_INSTANCE);

    data->console = hwnd;

    data->dialog = CreateDialog(hInst, MAKEINTRESOURCE(RFIDDialog),
                        hwnd, rfid_wnd_proc);
    SetWindowLongPtr(data->dialog, (LONG)data);

    for (y = 0; y < 24; y++) {
        for (x = 0; x <= 80; x++) {
            data->screen[y][x] = (x == 80) ? '\0' : ' ';
        }
    }
	_tcsncpy(data->screen[0], TEXT("RFID Reader"), 11);
	data->screen[0][11] = 0;
	data->screenrow = 1;

	e->emulator_data = data;

	return e;
}

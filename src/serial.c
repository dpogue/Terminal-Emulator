/**
 * @filename serial.c
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 10 20
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the function implementations for serial port
 * communication.
 */
#include "serial.h"

/**
 * Initialises a serial port handle for reading and writing
 *
 * @param LPCTSTR port  The name of the serial port to open.
 * @param HANDLE* fd    The pointer to the handle which will be initialised to the open
 *                      serial port connection.
 * @returns int 0 on success, >0 otherwise
 */
int OpenPort(const LPCTSTR port, HANDLE* fd, HWND hwnd) {
    COMMPROP cprops;
    DCB dcb;
    COMMCONFIG config;

    /* Create the file descriptor handle */
    if ((*fd = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
        == INVALID_HANDLE_VALUE) {
        return 1;
    }

    if (!GetCommProperties(*fd, &cprops)) {
        return 2;
    }

    /* Setup the port for sending and receiving data */
    if (!SetupComm(*fd, cprops.dwMaxRxQueue, cprops.dwMaxTxQueue)) {
        return 3;
    }

    /* Get the current DCB settings */
    if (!GetCommState(*fd, &dcb)) {
        return 4;
    }

    config.dwSize = sizeof(COMMCONFIG);
    config.wVersion = 0x1;
    config.dcb = dcb;
    config.dwProviderSubType = cprops.dwProvSubType;
    config.dwProviderOffset = 0;
    config.dwProviderSize = 0;

    /* Show the port configuration dialog */
    if (!CommConfigDialog(port, hwnd, &config)) {
        return 5;
    }

    /* Set the DCB to the config settings */
    if (!SetCommState(*fd, &config.dcb)) {
        return 6;
    }

    /* Specify events to receive */
    if (!SetCommMask(*fd, EV_RXCHAR | EV_TXEMPTY)) {
        return 7;
    }
    
    return 0;
}

/**
 * Sends data out the serial port pointed to by the handle fd.
 *
 * @param HANDLE fd The handle to the serial port.
 * @param LPVOID tx The data to be transmitted.
 * @param DWORD len The length of the data.
 *
 * @returns 0 if successful, greater than 0 otherwise.
 */
int SendData(HANDLE* fd, LPVOID tx, DWORD len) {
    DWORD dwEvtMask = 0;
    DWORD dwWait = 0;
    DWORD written = 0;
    OVERLAPPED ov;

    /* Initialise OVERLAPPED structure with defaults */
    ov.Internal = 0;
    ov.InternalHigh = 0;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!ov.hEvent) {
        return 1;
    }

    /* Write to the port handle */
    if (!WriteFile(*fd, tx, len, &written, &ov)) {
        /* Wait for the write to finish */
        WaitForSingleObject(ov.hEvent, INFINITE);
    }

    CloseHandle(ov.hEvent);
    return 0;
}

/**
 * Reads data from the serial port.
 *
 * @param HANDLE fd   The handle to the serial port
 * @param HWND hwnd   The handle to the application window
 *
 * @returns 0 if successful, greater than 0 otherwise
 */
int ReadData(HANDLE* fd, HWND hwnd) {
    DWORD dwEvtMask = 0;
    DWORD dwWait = 0;
    DWORD read = 0;
    OVERLAPPED ov;

    /* Initialise OVERLAPPED structure with defaults */
    ov.Internal = 0;
    ov.InternalHigh = 0;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!ov.hEvent) {
        return 1;
    }

    /* Wait for an event (characters to read) */
    if (!WaitCommEvent(*fd, &dwEvtMask, &ov)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            return 2;
        }
    }

    /* Wait for the characters to become available */
    dwWait = WaitForSingleObject(ov.hEvent, INFINITE);
    switch (dwWait) {
        case WAIT_OBJECT_0:
            if (dwEvtMask & EV_RXCHAR) {
                COMSTAT cstat;
                DWORD i;

                /* Get the stats (including number of available characters) */
                ClearCommError(*fd, NULL, &cstat);

                if (cstat.cbInQue > 0) {
                    DWORD bytes = cstat.cbInQue;
                    do {
                        BYTE* chars = (BYTE*)malloc(cstat.cbInQue + 1);
                        for (i = 0; i < (bytes > 1024 ? 1024 : bytes); i++) {
                            chars[i] = 0;
                            /* Read each character individually */
                            if (!ReadFile(*fd, (LPVOID)(chars + i), 1, &read, &ov)) {
                                free(chars);
                                return 3;
                            }
                        }
                        chars[i] = 0;

                        SendMessage(hwnd, TWM_RXDATA, (WPARAM)chars, cstat.cbInQue);
                        bytes -= (bytes > 1024 ? 1024 : bytes);
                    } while(bytes > 0);
                }
                ResetEvent(ov.hEvent);
            }
            break;
    }

    return 0;
}

/**
 * Closes a serial port handle.
 *
 * @param HANDLE* fd    The pointer to the handle of the serial port.
 * @return zero if successful, non-zero otherwise.
 */
int ClosePort(HANDLE* fd) {
    if(!CloseHandle(*fd)) {
        return 1;
    }

    return 0;
}

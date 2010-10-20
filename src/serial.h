/**
 * @filename serial.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 10 20
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the definitions and prototypes for communication
 * across a serial port.
 */
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <Windows.h>
#include <tchar.h>
#include "defines.h"

/**
 * Initialises a serial port handle for reading and writing
 * @implementation serial.c
 */
int OpenPort(const LPCTSTR port, HANDLE* fd, HWND hwnd);

/**
 * Sends data out the serial port pointed to by the handle fd.
 * @implementation serial.c
 */
int SendData(HANDLE* fd, LPVOID tx, DWORD len);

/**
 * Receives data from the serial port pointed to by the handle fd.
 * @implementation serial.c
 */
int ReadData(HANDLE* fd, HWND hwnd);

/**
 * Closes a serial port handle.
 * @implementation serial.c
 */
int ClosePort(HANDLE* fd);

#endif

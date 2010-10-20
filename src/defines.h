/**
 * @filename defines.h
 * @author Darryl Pogue
 * @designer Darryl Pogue
 * @date 2010 10 20
 * @project Terminal Emulator (COMP3980 Asn2)
 *
 * This file contains the definitions needed by all layers of the terminal.
 */
#ifndef _DEFINES_H_
#define _DEFINES_H_

#define APPNAME TEXT("Terminal Emulator")

/* APPLICATION MESSAGE ID DEFINES */
#define TWM_RXDATA (WM_APP + 1)

typedef struct _emulator Emulator;
typedef struct _TermInfo TermInfo;

#endif

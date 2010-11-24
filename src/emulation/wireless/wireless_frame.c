/**
 * @filename wireless_frame.cpp
 * @author Terence Stenvold
 * @designer
 * @date 2010 11 22
 * @project Terminal Emulator
 *
 * This file contains the prototypes and structures for data frames for the
 * wireless protocol design.
 */

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "wireless_frame.h"

WORD read_file(WirelessData *data, WirelessFrame *wf)
{
	WORD size;

	memset(wf->data,'\0', READ_SIZE);
	size = fread(wf->data,1,READ_SIZE, data->send.fd);

    if (size < READ_SIZE) {
        fclose(data->send.fd);
        data->send.fd= NULL;
    }

	return size;
}

crc crc_calculate(WirelessFrame *wf)
{
	crcInit();
	return crcFast((unsigned char*)wf,CRC_SIZE);
}

WirelessFrame* build_frame(WirelessData *data)
{
	WirelessFrame *wf = (WirelessFrame*)malloc(sizeof(WirelessFrame));

	wf->start = SOF;
    wf->sequence = data->send.sequence;
	wf->size = read_file(data, wf); /*wf->data is set in this function*/
	wf->crc = crc_calculate(wf);

	return wf;
}

BOOLEAN verify_frame(WirelessFrame* frm) {
    BYTE calc = crc_calculate(frm);

    return (calc == frm->crc);
}
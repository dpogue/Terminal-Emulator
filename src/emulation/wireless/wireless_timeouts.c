#include "wireless.h"
#include "../../terminal.h"

VOID CALLBACK WaitFrameACKTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    KillTimer(hwnd, timer);

    if (++data->counters[timer] > 3) {
        /* We have a problem here */
        OutputDebugString(TEXT("Got 3 timeouts! Quitting!"));
        return;
    } else {
        WirelessFrame* frame = (WirelessFrame*)malloc(sizeof(WirelessFrame));
        memcpy(frame, data->send.frame, sizeof(WirelessFrame));

        data->state = kSendingState;
        SendMessage(hwnd, TWM_TXDATA, (WPARAM)frame, sizeof(WirelessFrame));
        data->state = kWaitFrameACKState;

        SetTimer(hwnd, data->timeout, 20000, &WaitFrameACKTimeout);
    }
}
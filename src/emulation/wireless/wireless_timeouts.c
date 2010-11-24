#include "wireless.h"
#include "../../terminal.h"

VOID CALLBACK WaitFrameACKTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    KillTimer(hwnd, timer);

    if (++data->counters[timer] > 3) {
        /* We have a problem here */
        return;
    }

    data->state = kSendingState;
    SendMessage(hwnd, TWM_TXDATA, (WPARAM)data->send.frame, sizeof(WirelessFrame));
    data->state = kWaitFrameACKState;

    SetTimer(hwnd, data->timeout, 2000, &WaitFrameACKTimeout);
}
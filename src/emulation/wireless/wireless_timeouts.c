#include "wireless.h"
#include "../../terminal.h"

VOID CALLBACK SentENQTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    if (timer != kSentENQTimer)
        return;

    KillTimer(hwnd, timer);
    data->timeout = 0;

    if (++data->counters[timer] > 3) {
        /* We have a problem here */
        OutputDebugString(TEXT("Got 3 timeouts! [kSentENQ]\n"));
        data->state = kIdleState;
        return;
    } else {
        UINT rand_timer = rand() * (RAND_MAX + 1) % 5000 + 1000;
        data->state = kIdleState;
        data->timeout = SetTimer(hwnd, kRandDelayTimer, rand_timer, &RandDelayTimeout);
    }
}

VOID CALLBACK WaitFrameACKTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    if (timer != kWaitFrameACKTimer)
        return;

    KillTimer(hwnd, timer);
    data->timeout = 0;

    if (++data->counters[timer] > 3) {
        /* We have a problem here */
        OutputDebugString(TEXT("Got 3 timeouts! [kWaitFrameACK]\n"));
        data->state = kIdleState;
        return;
    } else {
        WirelessFrame* frame = (WirelessFrame*)malloc(sizeof(WirelessFrame));
        memcpy(frame, data->send.frame, sizeof(WirelessFrame));

        data->state = kSendingState;
        SendMessage(hwnd, TWM_TXDATA, (WPARAM)frame, sizeof(WirelessFrame));
        data->state = kWaitFrameACKState;

        data->timeout = SetTimer(hwnd, kWaitFrameACKTimer, 20000, &WaitFrameACKTimeout);
    }
}

VOID CALLBACK RandDelayTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    if (timer != kRandDelayTimer)
        return;

    KillTimer(hwnd, timer);
    data->timeout = 0;

    SendByte(data->hwnd, ENQ);
    data->state = kSentENQState;

    data->timeout = SetTimer(hwnd, kSentENQTimer, 10000, &SentENQTimeout);
}
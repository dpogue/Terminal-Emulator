#include "wireless.h"
#include "../../terminal.h"

VOID CALLBACK SentENQTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    if (timer != kSentENQTimer)
        return;

    KillTimer(hwnd, timer);
    data->timeout = 0;

  //  if (++data->counters[timer] >= 3) {
  //      /* We have a problem here */
  //      OutputDebugString(TEXT("Got 3 timeouts! [kSentENQ]\n"));
  //      data->state = kIdleState;
		//SetClassLong(data->hDlg, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
  //      return;
  //  } else {
    {
        UINT rand_timer = rand() * (RAND_MAX + 1) % TO_RAND_MAX + TO_RAND_MIN;
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

    if (++data->counters[timer] >= 3) {
        UINT rand_timer = rand() * (RAND_MAX + 1) % TO_RAND_MAX + TO_RAND_MIN;
        /* We have a problem here */
        OutputDebugString(TEXT("Got 3 timeouts! [kWaitFrameACK]\n"));
        MessageBox(hwnd, TEXT("Got 3 timeouts sending a frame"), TEXT("ERROR"), 0);
		//fclose(data->send.fd);
		//SetClassLong(data->hDlg, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
        data->state = kIdleState;
        data->timeout = SetTimer(hwnd, kRandDelayTimer, rand_timer, &RandDelayTimeout);
        return;
    } else {
        WirelessFrame* frame = (WirelessFrame*)malloc(sizeof(WirelessFrame));
        TCHAR* dbgmsg = (TCHAR*)malloc(sizeof(TCHAR) * 24);
        memcpy(frame, data->send.frame, sizeof(WirelessFrame));

        StringCchPrintf(dbgmsg, 24, TEXT("Resent frame %d.\n"), frame->sequence);
        OutputDebugString(dbgmsg);
        free(dbgmsg);

        data->state = kSendingState;
        SendMessage(hwnd, TWM_TXDATA, (WPARAM)frame, sizeof(WirelessFrame));
        data->state = kWaitFrameACKState;

        data->timeout = SetTimer(hwnd, kWaitFrameACKTimer, TO_FRAME, &WaitFrameACKTimeout);
    }
}

VOID CALLBACK ReadFrameTimeout(HWND hwnd, UINT msg, UINT_PTR timer, DWORD time) {
    TermInfo* ti = (TermInfo*)GetWindowLongPtr(hwnd, 0);
    WirelessData* data = (WirelessData*)ti->hEmulator[ti->e_idx]->emulator_data;

    if (timer != kReadFrameTimer)
        return;

    KillTimer(hwnd, timer);
    data->timeout = 0;

    if (++data->counters[timer] >= 3) {
        /* We have a problem here */
        OutputDebugString(TEXT("Got 3 timeouts! [kReadFrame]\n"));
		//fclose(data->read.fd);
		SetClassLong(data->hDlg, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
        data->state = kIdleState;
        return;
    } else {
        data->state = kReadFrameState;
        data->timeout = SetTimer(hwnd, kReadFrameTimer, TO_READ, &ReadFrameTimeout);
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

    data->timeout = SetTimer(hwnd, kSentENQTimer, TO_ENQ, &SentENQTimeout);
}
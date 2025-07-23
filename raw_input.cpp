// raw_input.cpp
// A native C++ DLL to capture raw keyboard input from multiple keyboards
// Exported as a DLL

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <vector>
#include <map>

// Struct to hold key event data
struct KeyEvent {
    HANDLE deviceHandle;
    USHORT vKey;
    USHORT makeCode;
    BOOL isKeyDown;
};

static std::vector<KeyEvent> g_keyEvents;
static HWND g_hwnd = nullptr;
static HINSTANCE g_hInstance = nullptr;

extern "C" __declspec(dllexport) void ClearEvents() {
    g_keyEvents.clear();
}

extern "C" __declspec(dllexport) int GetEventCount() {
    return static_cast<int>(g_keyEvents.size());
}

extern "C" __declspec(dllexport) BOOL GetEvent(int index, KeyEvent* outEvent) {
    if (index < 0 || index >= (int)g_keyEvents.size()) return FALSE;
    *outEvent = g_keyEvents[index];
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT) {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        std::vector<BYTE> lpb(dwSize);

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
            RAWINPUT* raw = (RAWINPUT*)lpb.data();
            if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                RAWKEYBOARD& kbd = raw->data.keyboard;

                KeyEvent evt;
                evt.deviceHandle = raw->header.hDevice;
                evt.vKey = kbd.VKey;
                evt.makeCode = kbd.MakeCode;
                evt.isKeyDown = !(kbd.Flags & RI_KEY_BREAK);

                g_keyEvents.push_back(evt);
            }
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

extern "C" __declspec(dllexport) BOOL StartRawInputCapture() {
    const wchar_t CLASS_NAME[] = L"RawInputCaptureWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) return FALSE;

    g_hwnd = CreateWindowEx(0, CLASS_NAME, L"", 0,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, g_hInstance, NULL);
    if (!g_hwnd) return FALSE;

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = g_hwnd;

    return RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

extern "C" __declspec(dllexport) void PumpMessages() {
    MSG msg;
    while (PeekMessage(&msg, g_hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        g_hInstance = hModule;
    }
    return TRUE;
}

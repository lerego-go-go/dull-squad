#include "imgui.h"
#include "imgui_impl_win32.h"

#include "ModuleUtils.hpp"

#include <windows.h>
#include <tchar.h>
#include <dwmapi.h>

#include <cstdio>
#include <cstdint>

#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD 1

typedef BOOL (WINAPI* tQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount);
tQueryPerformanceCounter _QueryPerformanceCounter = nullptr;

typedef BOOL (WINAPI* tQueryPerformanceFrequency)(LARGE_INTEGER* lpFrequency);
tQueryPerformanceFrequency _QueryPerformanceFrequency = nullptr;

typedef HCURSOR (WINAPI* tLoadCursorA)(HINSTANCE hInstance, LPCSTR lpCursorName);
tLoadCursorA _LoadCursorA = nullptr;

typedef HCURSOR (WINAPI* tSetCursor)(HCURSOR hCursor);
tSetCursor _SetCursor = nullptr;

typedef BOOL (WINAPI* tClientToScreen)(HWND hWnd, LPPOINT lpPoint);
tClientToScreen _ClientToScreen = nullptr;

typedef BOOL (WINAPI* tScreenToClient)(HWND hWnd, LPPOINT lpPoint);
tScreenToClient _ScreenToClient = nullptr;

typedef BOOL (WINAPI* tSetCursorPos)(int X, int Y);
tSetCursorPos _SetCursorPos = nullptr;

typedef BOOL (WINAPI* tGetCursorPos)(LPPOINT lpPoint);
tGetCursorPos _GetCursorPos = nullptr;

typedef HWND (WINAPI* tGetCapture)(VOID);
tGetCapture _GetCapture = nullptr;

typedef HWND (WINAPI* tSetCapture)(HWND hWnd);
tSetCapture _SetCapture = nullptr;

typedef BOOL (WINAPI* tReleaseCapture)(VOID);
tReleaseCapture _ReleaseCapture = nullptr;

typedef BOOL (WINAPI* tfTrackMouseEvent)(LPTRACKMOUSEEVENT lpEventTrack);
tfTrackMouseEvent _fTrackMouseEvent = nullptr;

typedef BOOL (WINAPI* tIsChild)(HWND hWndParent, HWND hWnd);
tIsChild _IsChild = nullptr;

typedef HWND (WINAPI* tGetForegroundWindow)(VOID);
tGetForegroundWindow _GetForegroundWindow = nullptr;

typedef BOOL (WINAPI* tGetClientRect)(HWND hWnd, LPRECT lpRect);
tGetClientRect _GetClientRect = nullptr;

static void* im_memset(void* destination, int value, size_t size)
{
    auto data = static_cast<uint8_t*>(destination);

    __stosb(data, static_cast<uint8_t>(value), size);
    return static_cast<void*>(data);
}

struct ImGui_ImplWin32_Data
{
    HWND                        hWnd;
    HWND                        MouseHwnd;
    bool                        MouseTracked;
    INT64                       Time;
    INT64                       TicksPerSecond;
    ImGuiMouseCursor            LastMouseCursor;
    bool                        HasGamepad;
    bool                        WantUpdateHasGamepad;

    ImGui_ImplWin32_Data()      { im_memset(this, 0, sizeof(*this)); }
};

static ImGui_ImplWin32_Data* ImGui_ImplWin32_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplWin32_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;
}

bool ImGui_ImplWin32_InitData()
{
    LPCSTR lpKernel32 = xor ("kernel32.dll");
    HMODULE hKernel32 = (HMODULE)ModuleUtils::GetModuleBase(lpKernel32);

    if (!hKernel32)
        return false;

    _QueryPerformanceCounter = (tQueryPerformanceCounter)ModuleUtils::GetFuncAddress(hKernel32, xor ("QueryPerformanceCounter"));
    _QueryPerformanceFrequency = (tQueryPerformanceFrequency)ModuleUtils::GetFuncAddress(hKernel32, xor ("QueryPerformanceFrequency"));

    LPCSTR lpUser32 = xor ("user32.dll");
    HMODULE hUser32 = (HMODULE)ModuleUtils::GetModuleBase(lpUser32);

    if (!hUser32)
        return false;

    _LoadCursorA = (tLoadCursorA)ModuleUtils::GetFuncAddress(hUser32, xor ("LoadCursorA"));
    _SetCursor = (tSetCursor)ModuleUtils::GetFuncAddress(hUser32, xor ("SetCursor"));

    _ClientToScreen = (tClientToScreen)ModuleUtils::GetFuncAddress(hUser32, xor ("ClientToScreen"));
    _ScreenToClient = (tScreenToClient)ModuleUtils::GetFuncAddress(hUser32, xor ("ScreenToClient"));

    _SetCursorPos = (tSetCursorPos)ModuleUtils::GetFuncAddress(hUser32, xor ("SetCursorPos"));
    _GetCursorPos = (tGetCursorPos)ModuleUtils::GetFuncAddress(hUser32, xor ("GetCursorPos"));

    _GetCapture = (tGetCapture)ModuleUtils::GetFuncAddress(hUser32, xor ("GetCapture"));
    _SetCapture = (tSetCapture)ModuleUtils::GetFuncAddress(hUser32, xor ("SetCapture"));
    _ReleaseCapture = (tReleaseCapture)ModuleUtils::GetFuncAddress(hUser32, xor ("ReleaseCapture"));

    _fTrackMouseEvent = (tfTrackMouseEvent)ModuleUtils::GetFuncAddress(hUser32, xor ("TrackMouseEvent"));

    _IsChild = (tIsChild)ModuleUtils::GetFuncAddress(hUser32, xor ("IsChild"));
    _GetForegroundWindow = (tGetForegroundWindow)ModuleUtils::GetFuncAddress(hUser32, xor ("GetForegroundWindow"));

    _GetClientRect = (tGetClientRect)ModuleUtils::GetFuncAddress(hUser32, xor ("GetClientRect"));

    return true;
}

// Functions
bool    ImGui_ImplWin32_Init(void* hwnd)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL);

    INT64 perf_frequency, perf_counter;
    if (!_QueryPerformanceFrequency((LARGE_INTEGER*)&perf_frequency))
        return false;
    if (!_QueryPerformanceCounter((LARGE_INTEGER*)&perf_counter))
        return false;

    // Setup backend capabilities flags
    ImGui_ImplWin32_Data* bd = IM_NEW(ImGui_ImplWin32_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = xor ("fbvx");
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    bd->hWnd = (HWND)hwnd;
    bd->WantUpdateHasGamepad = true;
    bd->TicksPerSecond = perf_frequency;
    bd->Time = perf_counter;
    bd->LastMouseCursor = ImGuiMouseCursor_COUNT;

    io.ImeWindowHandle = hwnd;

    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    //io.KeyMap[ImGuiKey_A] = 'A';
    //io.KeyMap[ImGuiKey_C] = 'C';
    //io.KeyMap[ImGuiKey_V] = 'V';
    //io.KeyMap[ImGuiKey_X] = 'X';
    //io.KeyMap[ImGuiKey_Y] = 'Y';
    //io.KeyMap[ImGuiKey_Z] = 'Z';

    return true;
}

void ImGui_ImplWin32_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();

    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    IM_DELETE(bd);
}

static bool ImGui_ImplWin32_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();

    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        _SetCursor(NULL);
    }
    else
    {
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        _SetCursor(_LoadCursorA(NULL, win32_cursor));
    }
    return true;
}

static void ImGui_ImplWin32_UpdateMousePos()
{
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(bd->hWnd != 0);

    const ImVec2 mouse_pos_prev = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    HWND focused_window = _GetForegroundWindow();
    HWND hovered_window = bd->MouseHwnd;
    HWND mouse_window = NULL;
    
    if (hovered_window && (hovered_window == bd->hWnd || _IsChild(hovered_window, bd->hWnd)))
        mouse_window = hovered_window;
    else if (focused_window && (focused_window == bd->hWnd || _IsChild(focused_window, bd->hWnd)))
        mouse_window = focused_window;
    
    if (mouse_window == NULL)
        return;

    if (io.WantSetMousePos)
    {
        POINT pos = { (int)mouse_pos_prev.x, (int)mouse_pos_prev.y };
        if (_ClientToScreen(bd->hWnd, &pos))
            _SetCursorPos(pos.x, pos.y);
    }

    POINT pos;
    if (_GetCursorPos(&pos) && _ScreenToClient(mouse_window, &pos))
        io.MousePos = ImVec2((float)pos.x, (float)pos.y);
}

static void ImGui_ImplWin32_UpdateGamepads()
{

}

void ImGui_ImplWin32_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();
    IM_ASSERT(bd != NULL);

    RECT rect = { 0, 0, 0, 0 };
    _GetClientRect(bd->hWnd, &rect);

    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    INT64 current_time = 0;
    _QueryPerformanceCounter((LARGE_INTEGER*)&current_time);

    io.DeltaTime = (float)(current_time - bd->Time) / bd->TicksPerSecond;
    bd->Time = current_time;

    ImGui_ImplWin32_UpdateMousePos();

    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (bd->LastMouseCursor != mouse_cursor)
    {
        bd->LastMouseCursor = mouse_cursor;
        ImGui_ImplWin32_UpdateMouseCursor();
    }

    ImGui_ImplWin32_UpdateGamepads();
}

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif

#if 0
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();

    switch (msg)
    {
    case WM_MOUSEMOVE:
        bd->MouseHwnd = hwnd;
        if (!bd->MouseTracked)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            _fTrackMouseEvent(&tme);
            bd->MouseTracked = true;
        }
        break;
    case WM_MOUSELEAVE:
        if (bd->MouseHwnd == hwnd)
            bd->MouseHwnd = NULL;
        bd->MouseTracked = false;
        break;
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
        if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        if (!ImGui::IsAnyMouseDown() && _GetCapture() == NULL)
            _SetCapture(hwnd);
        io.MouseDown[button] = true;
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if (msg == WM_LBUTTONUP) { button = 0; }
        if (msg == WM_RBUTTONUP) { button = 1; }
        if (msg == WM_MBUTTONUP) { button = 2; }
        if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        io.MouseDown[button] = false;
        if (!ImGui::IsAnyMouseDown() && _GetCapture() == hwnd)
            _ReleaseCapture();
        return 0;
    }
    case WM_MOUSEWHEEL:
        io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        bool down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        if (wParam < 256)
            io.KeysDown[wParam] = down;
        if (wParam == VK_CONTROL)
            io.KeyCtrl = down;
        if (wParam == VK_SHIFT)
            io.KeyShift = down;
        if (wParam == VK_MENU)
            io.KeyAlt = down;
        return 0;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        io.AddFocusEvent(msg == WM_SETFOCUS);
        return 0;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacterUTF16((unsigned short)wParam);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;
    case WM_DEVICECHANGE:
        if ((UINT)wParam == DBT_DEVNODES_CHANGED)
            bd->WantUpdateHasGamepad = true;
        return 0;
    }
    return 0;
}
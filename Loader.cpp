#include "UEUtils.hpp"

#include <d3d11.h>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#define M_PI 3.14159265358979323846

typedef SHORT (WINAPI* tGetAsyncKeyState)(int vKey);
tGetAsyncKeyState _GetAsyncKeyState = nullptr;

static HWND hwnd = 0;

ImFont* guiFont = nullptr;

ID3D11Device* Device = nullptr;

ID3D11DeviceContext* DeviceContext = nullptr;
ID3D11RenderTargetView* RenderTargetView = nullptr;

typedef HRESULT (__fastcall* tPresentScene)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
tPresentScene PresentScene_Original = NULL;

typedef HRESULT (__fastcall* tResizeBuffers)(IDXGISwapChain* SwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
tResizeBuffers ResizeBuffers_Original = NULL;

static BOOL ImMenu = FALSE;
static float ImAlpha = 255.f;

typedef LRESULT (CALLBACK* tWindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
tWindowProc WndProc_Original = nullptr;

typedef LRESULT (WINAPI* tCallWindowProcA)(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
tCallWindowProcA _CallWindowProcA = nullptr;

typedef LONG_PTR (WINAPI* tGetWindowLongPtrA)(HWND hWnd, int nIndex);
tGetWindowLongPtrA _GetWindowLongPtrA = nullptr;

typedef LONG_PTR (WINAPI* tSetWindowLongPtrA)(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
tSetWindowLongPtrA _SetWindowLongPtrA = nullptr;

typedef BOOL (WINAPI* tClipCursor)(CONST RECT* lpRect);
tClipCursor _ClipCursor = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_KEYDOWN && wParam == VK_HOME)
	{
		ImMenu = !ImMenu;

		return TRUE;
	}

	if (Msg == WM_KEYDOWN && wParam == VK_ESCAPE && ImMenu)
	{
		ImMenu = FALSE;
	}

	if (ImMenu)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
			return TRUE;

		if (Msg != WM_LBUTTONDOWN && Msg != WM_LBUTTONUP && Msg != WM_RBUTTONDOWN && Msg != WM_RBUTTONUP && Msg != WM_MOUSEWHEEL && Msg != WM_MOUSEMOVE && Msg != 0x20 && Msg != 0x8 && Msg != 0x21 && Msg != 0x7)
		{
			return _CallWindowProcA(WndProc_Original, hWnd, Msg, wParam, lParam);
		}

		return TRUE;
	}

	return _CallWindowProcA(WndProc_Original, hWnd, Msg, wParam, lParam);
}

int pages = 0;

__forceinline const char* GetKeyBind(int index)
{
	switch (index)
	{
	case 0: return xor ("Unknown");
	case 1: return xor ("LeftMouse");
	case 2: return xor ("RightMouse");
	case 3: return xor ("Cancel");
	case 4: return xor ("MiddleMouse");
	case 5: return xor ("X_Mouse");
	case 6: return xor ("X_Mouse2");
	case 7: return xor ("Unknown");
	case 8: return xor ("Back");
	case 9: return xor ("Tab");
	case 10: return xor ("Unknown");
	case 11: return xor ("Unknown");
	case 12: return xor ("Clear");
	case 13: return xor ("Return");
	case 14: return xor ("Unknown");
	case 15: return xor ("Unknown");
	case 16: return xor ("Shift");
	case 17: return xor ("Control");
	case 18: return xor ("Menu");
	case 19: return xor ("Pause");
	case 20: return xor ("CapsLock");
	case 22: return xor ("Unknown");
	case 24: return xor ("Final");
	case 26: return xor ("Unknown");
	case 27: return xor ("Escape");
	case 28: return xor ("Convert");
	case 29: return xor ("Nonconvert");
	case 30: return xor ("Accept");
	case 31: return xor ("Modechange");
	case 32: return xor ("Space");
	case 33: return xor ("PageUp");
	case 34: return xor ("PageDown");
	case 35: return xor ("End");
	case 36: return xor ("Home");
	case 37: return xor ("Left");
	case 38: return xor ("Up");
	case 39: return xor ("Right");
	case 40: return xor ("Down");
	case 41: return xor ("Select");
	case 42: return xor ("Print");
	case 43: return xor ("Execute");
	case 44: return xor ("Snapshot");
	case 45: return xor ("Insert");
	case 46: return xor ("Delete");
	case 47: return xor ("Help");
	case 48: return xor ("0");
	case 49: return xor ("1");
	case 50: return xor ("2");
	case 51: return xor ("3");
	case 52: return xor ("4");
	case 53: return xor ("5");
	case 54: return xor ("6");
	case 55: return xor ("7");
	case 56: return xor ("8");
	case 57: return xor ("9");
	case 58: return xor ("Unknown");
	case 59: return xor ("Unknown");
	case 60: return xor ("Unknown");
	case 61: return xor ("Unknown");
	case 62: return xor ("Unknown");
	case 63: return xor ("Unknown");
	case 64: return xor ("Unknown");
	case 65: return xor ("A");
	case 66: return xor ("B");
	case 67: return xor ("C");
	case 68: return xor ("D");
	case 69: return xor ("E");
	case 70: return xor ("F");
	case 71: return xor ("G");
	case 72: return xor ("H");
	case 73: return xor ("I");
	case 74: return xor ("J");
	case 75: return xor ("K");
	case 76: return xor ("L");
	case 77: return xor ("M");
	case 78: return xor ("N");
	case 79: return xor ("O");
	case 80: return xor ("P");
	case 81: return xor ("Q");
	case 82: return xor ("R");
	case 83: return xor ("S");
	case 84: return xor ("T");
	case 85: return xor ("U");
	case 86: return xor ("V");
	case 87: return xor ("W");
	case 88: return xor ("X");
	case 89: return xor ("Y");
	case 90: return xor ("Z");
	case 91: return xor ("LeftWin");
	case 92: return xor ("RightWin");
	case 93: return xor ("Apps");
	case 94: return xor ("Unknown");
	case 95: return xor ("Sleep");
	case 96: return xor ("NumPad_0");
	case 97: return xor ("NumPad_1");
	case 98: return xor ("NumPad_2");
	case 99: return xor ("NumPad_3");
	case 100: return xor ("NumPad_4");
	case 101: return xor ("NumPad_5");
	case 102: return xor ("NumPad_6");
	case 103: return xor ("NumPad_7");
	case 104: return xor ("NumPad_8");
	case 105: return xor ("NumPad_9");
	case 106: return xor ("Multiply");
	case 107: return xor ("Add");
	case 108: return xor ("Separator");
	case 109: return xor ("Subtract");
	case 110: return xor ("Decimal");
	case 111: return xor ("Divide");
	case 112: return xor ("F1");
	case 113: return xor ("F2");
	case 114: return xor ("F3");
	case 115: return xor ("F4");
	case 116: return xor ("F5");
	case 117: return xor ("F6");
	case 118: return xor ("F7");
	case 119: return xor ("F8");
	case 120: return xor ("F9");
	case 121: return xor ("F10");
	case 122: return xor ("F11");
	case 123: return xor ("F12");
	case 124: return xor ("F13");
	case 125: return xor ("F14");
	case 126: return xor ("F15");
	case 127: return xor ("F16");
	case 128: return xor ("F17");
	case 129: return xor ("F18");
	case 130: return xor ("F19");
	case 131: return xor ("F20");
	case 132: return xor ("F21");
	case 133: return xor ("F22");
	case 134: return xor ("F23");
	case 135: return xor ("F24");
	case 136: return xor ("Unknown");
	case 137: return xor ("Unknown");
	case 138: return xor ("Unknown");
	case 139: return xor ("Unknown");
	case 140: return xor ("Unknown");
	case 141: return xor ("Unknown");
	case 142: return xor ("Unknown");
	case 143: return xor ("Unknown");
	case 144: return xor ("NumLock");
	case 145: return xor ("ScrollLock");
	case 151: return xor ("Unknown");
	case 152: return xor ("Unknown");
	case 153: return xor ("Unknown");
	case 154: return xor ("Unknown");
	case 155: return xor ("Unknown");
	case 156: return xor ("Unknown");
	case 157: return xor ("Unknown");
	case 158: return xor ("Unknown");
	case 159: return xor ("Unknown");
	case 160: return xor ("LeftShift");
	case 161: return xor ("RightShift");
	case 162: return xor ("LeftCtrl");
	case 163: return xor ("RightCtrl");
	case 164: return xor ("LeftAlt");
	case 165: return xor ("RightAlt");

	default:
	{
		return xor ("Unknown");
	}
	}
};

__forceinline bool HotKey(const char* label, const char* guiId, int* k)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;

	const ImGuiStyle& style = g.Style;
	const ImVec2 CursorPos = window->DC.CursorPos;

	const ImGuiID id = window->GetID(guiId);
	const ImVec2 label_size = g.Font->CalcTextSizeA(g.Font->FontSize, FLT_MAX, 0.0f, label);

	const ImRect frame_bb =
	{
		ImVec2(CursorPos.x, CursorPos.y),
		ImVec2(CursorPos.x + ImGui::GetWindowWidth() - 20,CursorPos.y + 20)
	};

	const ImRect total_bb(CursorPos, frame_bb.Max);

	ImGui::ItemSize(total_bb, style.FramePadding.y);

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	if (hovered)
	{
		ImGui::SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (user_clicked)
	{
		if (g.ActiveId != id)
		{
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*k = 0;
		}
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0])
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	bool value_changed = false;
	int key = *k;

	if (g.ActiveId == id)
	{
		for (auto i = 0; i < 5; i++)
		{
			if (io.MouseDown[i])
			{
				switch (i)
				{
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}

		if (!value_changed)
		{
			for (auto i = VK_BACK; i <= VK_RMENU; i++)
			{
				if (io.KeysDown[i])
				{
					key = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
		{
			*k = 0;
			ImGui::ClearActiveID();
		}
		else
			*k = key;
	}

	char buf_display[64];
	CRT::StrCpy(buf_display, xor ("None"));

	if (*k != 0 && g.ActiveId != id)
		CRT::StrCpy(buf_display, GetKeyBind(*k));
	else if (g.ActiveId == id)
		CRT::StrCpy(buf_display, xor ("Press a key"));

	static float alpha = 1.f;

	const ImVec2 ActiveKeyPos =
	{
		frame_bb.Max.x - 10 - 160 / 2.f - g.Font->CalcTextSizeA(18,FLT_MAX, 0.0f, buf_display).x / 2.f,
		frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y) / 2.f - g.Font->CalcTextSizeA(18,FLT_MAX, 0.0f, buf_display).y / 2.f,
	};

	window->DrawList->AddRectFilled({ frame_bb.Max.x - 170,frame_bb.Max.y - 20 }, { frame_bb.Max.x - 10,frame_bb.Max.y }, ImGui::GetColorU32(ImGuiCol_FrameBg, 1.0f), 8);
	window->DrawList->AddRect({ frame_bb.Max.x - 170,frame_bb.Max.y - 20 }, { frame_bb.Max.x - 10,frame_bb.Max.y }, ImColor(55, 55, 55), 8);

	auto kek = ImGui::GetColorU32(ImGuiCol_Text, alpha);

	window->DrawList->AddText(ActiveKeyPos, kek, buf_display);

	if (label_size.x > 0)
		window->DrawList->AddText(g.Font, 16, { total_bb.Min.x - 10,total_bb.Min.y + (total_bb.Max.y - total_bb.Min.y) / 2 - label_size.y / 2 }, ImColor(100, 100, 100), label);

	return value_changed;
}

__forceinline bool TabButton(const char* label, int page)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;

	const ImRect bb({ pos.x + 12,pos.y }, { pos.x + ImGui::GetWindowWidth() - 22,pos.y + 23 });
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	bool active = pages == page;

	static int alpha = 0;
	if (active)
	{
		alpha += 5;
		if (alpha >= 255)
			alpha = 255;
	}

	if (pressed && !active)
		alpha = 0;

	static float alphas = 0.f;
	if (hovered)
	{
		alphas += 5.f;
		if (alphas >= 255.f)
			alphas = 255.f;
	}

	static int lastHovePage = page;

	if (hovered && lastHovePage != page)
	{
		alphas = 100.0f;
		lastHovePage = page;
	}

	if (active)
	{
		auto d = ImGui::GetWindowDrawList();

		d->AddRectFilledMultiColor(bb.Min, bb.Max, ImColor(40, 250, 89, alpha / 5), ImColor(40, 250, 89, alpha / 5), ImColor(40, 250, 89, alpha / 50), ImColor(40, 250, 89, alpha / 50));
		d->AddRectFilled(ImVec2(bb.Min.x - 2, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), ImColor(40, 250, 89, alpha));
	}

	const ImU32 bg_col = ImGui::GetColorU32(ImGuiCol_TabActive);

	ImColor col = active ? ImColor(0, 255, 60, alpha) : hovered ? ImColor(171.f / 255.f, 171.f / 255.f, 171.f / 255.f, alphas / 255.f) : ImColor(bg_col);

	ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)col);

	window->DrawList->AddText(g.Font, g.FontSize, { bb.Min.x + ((bb.Max.x - bb.Min.x) / 2) - label_size.x / 2,bb.Min.y + ((bb.Max.y - bb.Min.y) / 2) - label_size.y / 2 },
		col, label);

	ImGui::PopStyleColor();

	if (pressed)
		pages = page;

	return pressed;
}

static int AimBotKey = VK_RBUTTON;

static int AimBotFOV = 15;
static int AimBotSmooth = 0;
static int AimBotDistance = 700;

static int AimBone = 1;
static int BoxStyle = 2;

static bool AimBotEnabled = false, AimOnVisibles = false, AimLock = false, DrawFOV = false;
static bool PlayerESP = false, BoneESP = false, DistanceESP = false, NicknameESP = false, WeaponESP = false, HealthBar = false;

static bool SpeedEnabled = false;
static int SpeedKey = VK_LMENU;
static int SpeedRatio = 5;

static bool bNoRecoil = false, bNoSpread = false, bNoSway = false;

static __forceinline auto GetAimBone() -> int32_t
{
	auto Bone = 13;

	if (AimBone == 0)
		Bone = 2;
	else if (AimBone == 1)
		Bone = 13;
	else if (AimBone == 2)
		Bone = 6;

	return Bone;
}

__forceinline VOID DrawImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	_ClipCursor(0);

	ImGui::NewFrame();
	{
		ImGui::SetNextWindowPos(ImVec2(520, 110), ImGuiCond_FirstUseEver);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 44.f / 255.f, 44.f / 255.f, 44.f / 255.f, ImAlpha / 255.f });
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 40.f / 255.f, 250.f / 255.f, 89.f / 255.f, 200.f / 255.f });
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, { 40.f / 255.f, 250.f / 255.f, 89.f / 255.f, ImAlpha / 255.f });

		ImGuiStyle& s = ImGui::GetStyle();
		auto c = s.Colors;

		c[ImGuiCol_Header] = ImColor(44.f / 255.f, 44.f / 255.f, 44.f / 255.f, ImAlpha / 255.f);//ImColor(44, 44, 44);
		c[ImGuiCol_HeaderHovered] = ImColor(44, 44, 44, 200);
		c[ImGuiCol_HeaderActive] = ImColor(74, 74, 74);
		c[ImGuiCol_CheckMark] = ImColor(40, 250, 89);
		c[ImGuiCol_TabActive] = ImColor(147.f / 255.f, 147.f / 255.f, 147.f / 255.f, ImAlpha / 255.f);
		c[ImGuiCol_ButtonActive] = ImColor(255.f / 255.f, 0.f / 255.f, 0.f / 255.f, ImAlpha / 255.f);
		c[ImGuiCol_ButtonHovered] = ImColor(201.f / 255.f, 201.f / 255.f, 201.f / 255.f, ImAlpha / 255.f);
		c[ImGuiCol_SliderGrabActive] = ImColor(255.f / 255.f, 255.f / 255.f, 255.f / 255.f, ImAlpha / 255.f);
		c[ImGuiCol_TextDisabled] = ImColor(95, 98, 103);

		//ImGuiCol_CheckMark

		ImGui::PushStyleColor(ImGuiCol_Text, { 100.f / 255.f,100.f / 255.f, 100.f / 255.f, ImAlpha / 255.f });

		ImGui::SetNextWindowBgAlpha(0.87f);
		ImGui::Begin(xor ("GCG2CF"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		{
			ImGui::SetWindowSize({ 420,297 });
			auto* render = ImGui::GetWindowDrawList();
			auto* window = ImGui::GetCurrentWindow();
			auto What = *GImGui;
			const auto PosStart = window->Pos;
			const auto WindowsSize = window->Size;

			// draw zapolnenie v gui

			if (ImMenu)
			{
				ImAlpha += 5.f;
				if (ImAlpha >= 255.f)
					ImAlpha = 255.f;
			}

			render->AddRectFilled(PosStart, { PosStart.x + WindowsSize.x,PosStart.y + WindowsSize.y }, ImColor(18.f / 255.f, 18.f / 255.f, 18.f / 255.f, ImAlpha / 255.f), 18);
			render->AddRectFilled(PosStart, { PosStart.x + 121,PosStart.y + 285 }, ImColor(23.f / 255.f, 23.f / 255.f, 23.f / 255.f, ImAlpha / 255.f), 18);

			ImGui::GetStyle().ScrollbarSize = 2;
			ImGui::PushStyleColor(ImGuiCol_Border, { 44.f / 255, 44.f / 255, 44.f / 255, ImAlpha / 255.f });

			ImGui::GetStyle().ChildRounding = 8;
			ImGui::BeginChild(xor ("DZVES"), { 121,205 });
			{
				ImGui::SetCursorPos({ ImGui::GetCursorPos().x,ImGui::GetCursorPos().y + 15 });
				TabButton(xor ("AimBot"), 0);
				TabButton(xor ("Visual"), 1);
				TabButton(xor ("Misc"), 2);
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild(xor ("Page"));
			{
				ImGui::BeginChild(xor ("block1"), { 270,275 }, true);
				{
					switch (pages)
					{
					case 0:
						ImGui::Checkbox(xor ("Enabled"), &AimBotEnabled);
						HotKey(xor ("Bind"), xor ("AiBind"), &AimBotKey);
						ImGui::Combo(xor ("Bone"), &AimBone, xor ("Body\0Head\0Neck\0"), 3);
						ImGui::Checkbox(xor ("DrawFOV"), &DrawFOV);
						ImGui::Checkbox(xor ("Invisibles"), &AimOnVisibles);

						ImGui::NewLine();
						ImGui::SliderInt(xor ("FOV"), &AimBotFOV, 0, 180);
						ImGui::SliderInt(xor ("Smooth"), &AimBotSmooth, 0, 20);
						ImGui::SliderInt(xor ("Distance"), &AimBotDistance, 0, 1200);

						break;
					case 1:

						ImGui::Checkbox(xor ("Player ESP"), &PlayerESP);
						ImGui::Combo(xor ("PStyle"), xor ("Style"), &BoxStyle, xor ("2D\0\\3D\0Corner\0"), 3);

						ImGui::Checkbox(xor ("Weapon"), &WeaponESP);
						ImGui::Checkbox(xor ("PSkelet"), xor ("Skeleton"), &BoneESP);
						ImGui::Checkbox(xor ("PDistance"), xor ("Distance"), &DistanceESP);
						ImGui::Checkbox(xor ("Nickname"), &NicknameESP);
						ImGui::Checkbox(xor ("Health Bar"), &HealthBar);

						break;
					case 2:
						ImGui::Checkbox(xor ("Speed"), &SpeedEnabled);
						HotKey(xor ("Bind"), xor ("SpBind"), &SpeedKey);
						ImGui::Checkbox(xor ("No Recoil"), &bNoRecoil);
						ImGui::Checkbox(xor ("No Spread"), &bNoSpread);
						ImGui::Checkbox(xor ("No Sway"), &bNoSway);

						ImGui::NewLine();
						ImGui::SliderInt(xor ("Multiplier"), &SpeedRatio, 1, 10);

						break;
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}

			ImGui::PopStyleColor();
			ImGui::End();
		}

		ImGui::EndFrame();
		ImGui::Render();
	}
}

VOID OnDrawGui()
{
	if (_GetWindowLongPtrA(hwnd, GWLP_WNDPROC) != (LONG_PTR)WndProc_Hook)
	{
		WndProc_Original = (tWindowProc)_SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc_Hook);
	}

	if (ImMenu)
	{
		DrawImGui();
	}
}

__forceinline ImGuiWindow* BeginScene()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });

		ImGui::Begin(xor ("Gg241"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
		{
			ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
		}
	}

	return ImGui::GetCurrentWindow();
}

__forceinline VOID EndScene(ImGuiWindow* window)
{
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::Render();
}

__forceinline bool DrawOutlinedText(ImFont* pFont, const string& text, const ImVec2& textPos, ImU32 color, float size, bool center, const ImVec2 jVec = ImVec2(0, 0))
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (!window)
		return false;

	int i = 0;

	auto outlineColor = ImGui::GetColorU32(ImVec4(jVec.x, jVec.y, jVec.y, 255));
	auto textColor = ImGui::GetColorU32(color);

	while (i != 1)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

		if (center)
		{
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) + 1, (textPos.y + textSize.y * i) + 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) - 1, (textPos.y + textSize.y * i) - 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) + 1, (textPos.y + textSize.y * i) - 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) - 1, (textPos.y + textSize.y * i) + 1), outlineColor, text.c_str());

			window->DrawList->AddText(pFont, size, ImVec2(textPos.x - textSize.x / (2.0f), textPos.y + textSize.y * i), textColor, text.c_str());
		}
		else
		{
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) + 1, (textPos.y + textSize.y * i) + 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) - 1, (textPos.y + textSize.y * i) - 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) + 1, (textPos.y + textSize.y * i) - 1), outlineColor, text.c_str());
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) - 1, (textPos.y + textSize.y * i) + 1), outlineColor, text.c_str());

			window->DrawList->AddText(pFont, size, ImVec2(textPos.x, textPos.y + textSize.y * i), textColor, text.c_str());
		}

		i++;
	}

	return true;
}

__forceinline bool DrawOutlinedTextW(ImFont* pFont, const wstring& text, const ImVec2& textPos, ImU32 color, float size, bool center, const ImVec2 jVec = ImVec2(0, 0))
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (!window)
		return false;

	char display[256] = { };
	ImTextStrToUtf8(display, ARRAYSIZE(display), reinterpret_cast<const ImWchar*>(text.c_str()), reinterpret_cast<const ImWchar*>(text.c_str() + text.length()));

	int i = 0;

	auto outlineColor = ImGui::GetColorU32(ImVec4(jVec.x, jVec.y, jVec.y, 255));
	auto textColor = ImGui::GetColorU32(color);

	while (i != 1)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, display);

		if (center)
		{
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) + 1, (textPos.y + textSize.y * i) + 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) - 1, (textPos.y + textSize.y * i) - 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) + 1, (textPos.y + textSize.y * i) - 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x - textSize.x / (2.0f)) - 1, (textPos.y + textSize.y * i) + 1), outlineColor, display);

			window->DrawList->AddText(pFont, size, ImVec2(textPos.x - textSize.x / (2.0f), textPos.y + textSize.y * i), textColor, display);
		}
		else
		{
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) + 1, (textPos.y + textSize.y * i) + 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) - 1, (textPos.y + textSize.y * i) - 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) + 1, (textPos.y + textSize.y * i) - 1), outlineColor, display);
			window->DrawList->AddText(pFont, size, ImVec2((textPos.x) - 1, (textPos.y + textSize.y * i) + 1), outlineColor, display);

			window->DrawList->AddText(pFont, size, ImVec2(textPos.x, textPos.y + textSize.y * i), textColor, display);
		}

		i++;
	}

	return true;
}

__forceinline void DrawCornerESP(float X, float Y, float W, float H, const ImU32& color, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (!window)
		return;

	float lineW = (W / 2);
	float lineH = (H / 2);

	auto bleckColor = ImGui::ColorConvertFloat4ToU32(ImVec4((1.0f) / (255.0f), (1.0f) / (255.0f), (1.0f) / (255.0f), (255.0f) / (255.0f)));
	auto boxColor = ImGui::GetColorU32(color);

	window->DrawList->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), bleckColor, 2);
	window->DrawList->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), bleckColor, 2);

	window->DrawList->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), boxColor, thickness);
	window->DrawList->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), boxColor, thickness);
}

__forceinline VOID DrawLine(Vector3 PositionToStart, Vector3 PositionToEnd, float Thickness, ImU32 Color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddLine(ImVec2(PositionToStart.x, PositionToStart.y), ImVec2(PositionToEnd.x, PositionToEnd.y), Color, Thickness);
}

__forceinline VOID DrawFilledRect(float x, float y, float w, float h, ImU32 color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (!window)
		return;

	auto bleckColor = ImGui::ColorConvertFloat4ToU32(ImVec4((1.0f) / (255.0f), (1.0f) / (255.0f), (1.0f) / (255.0f), (255.0f) / (255.0f)));

	window->DrawList->AddRectFilled(ImVec2(x, y - 1), ImVec2(x + w, y + h), bleckColor, 0, 0);
	window->DrawList->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + w, y + h), bleckColor, 0, 0);
	window->DrawList->AddRectFilled(ImVec2(x - 1, y), ImVec2(x + w, y + h), bleckColor, 0, 0);
	window->DrawList->AddRectFilled(ImVec2(x + 1, y), ImVec2(x + w, y + h), bleckColor, 0, 0);
	window->DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
}

__forceinline VOID DrawBorderBox(int x, int y, int x2, int y2, int thickness, ImU32 color)
{
	DrawFilledRect(x, y, x2, thickness, color);
	DrawFilledRect(x, y + y2, x2, thickness, color);
	DrawFilledRect(x, y, thickness, y2, color);
	DrawFilledRect(x + x2, y, thickness, y2 + thickness, color);
}

__forceinline VOID Draw2DBox(float x, float y, float width, float height, int thickness, ImU32 boxColor)
{
	DrawBorderBox(x, y, width, height, thickness, boxColor);
}

static float screenWidth = 0;
static float screenHeight = 0;

static float crossX = 0.0f, crossY = 0.0f;
static float cameraFov = 80.0f;

static Vector3 cameraLocation;
static Vector3 cameraRotation;

UClass* STATIC_Soldier()
{
	static UClass* object = 0;
	
	if (!object)
		object = (UClass*)UEUtils::FindObject(xor ("/Script/Squad.SQSoldier"));

	return object;
}

FCameraCacheEntry GetCameraCache(PVOID PlayerCameraManager)
{
	FCameraCacheEntry CameraCache = *reinterpret_cast<FCameraCacheEntry*>(reinterpret_cast<PBYTE>(PlayerCameraManager) + UEOffsets::Engine::PlayerCameraManager::CameraCachePrivate);

	return CameraCache;
}

VOID UpdateCameraInfo(PVOID PlayerCameraManager)
{
	auto CameraCache = GetCameraCache(PlayerCameraManager);
	auto MinimalViewInfo = CameraCache.POV;

	cameraFov = MinimalViewInfo.FOV;

	cameraLocation = MinimalViewInfo.Location;
	cameraRotation = MinimalViewInfo.Rotation;
}

template<class DataType>
class SpeedPerfomance
{
	double dSpeed;
	float jyjyk;
	DataType time_offset;
	DataType time_last_update;

public:
	SpeedPerfomance(DataType currentRealTime, double initialSpeed)
	{
		time_offset = currentRealTime;
		time_last_update = currentRealTime;

		dSpeed = initialSpeed;
	}

	void setSpeed(DataType currentRealTime, double speed)
	{
		time_offset = getCurrentTime(currentRealTime);
		time_last_update = currentRealTime;

		dSpeed = speed;
	}

	DataType getCurrentTime(DataType currentRealTime)
	{
		DataType difference = currentRealTime - time_last_update;

		return (DataType)(dSpeed * difference) + time_offset;
	}
};

static bool bSpeedHackInit = false;

const double kInitialSpeed = 1.0;

SpeedPerfomance<LONGLONG>  g_SpeedPerfomance(0, kInitialSpeed);

typedef BOOL(WINAPI* tQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount);
tQueryPerformanceCounter QueryPerformanceCounter_Original = NULL;

BOOL WINAPI QueryPerformanceCounter_Hook(LARGE_INTEGER* lpPerformanceCount)
{
	LARGE_INTEGER liCounter;

	BOOL Status = QueryPerformanceCounter_Original(&liCounter);

	lpPerformanceCount->QuadPart = g_SpeedPerfomance.getCurrentTime(liCounter.QuadPart);

	return Status;
}

void SetSpeed(double speed)
{
	LARGE_INTEGER performanceCounter;
	QueryPerformanceCounter_Original(&performanceCounter);

	g_SpeedPerfomance.setSpeed(performanceCounter.QuadPart, speed);
}

void CreateHook(__int64 iAddr, __int64 iFunction, __int64* iOriginal)
{
	static uintptr_t GameOverlayBase = 0;

	if (!GameOverlayBase)
	{
		LPCSTR lpGameOverlay = xor ("GameOverlayRenderer64.dll");
		GameOverlayBase = ModuleUtils::GetModuleBase(lpGameOverlay);
	}

	static uintptr_t pHookAddr;
	if (!pHookAddr)
		pHookAddr = (uintptr_t)ModuleUtils::PatternScanInModule(GameOverlayBase, xor ("48 ? ? ? ? 57 48 83 EC 30 33 C0"));

	auto hook = ((__int64(__fastcall*)(__int64 addr, __int64 func, __int64* orig, __int64 smthng))(pHookAddr));
	hook((__int64)iAddr, (__int64)iFunction, iOriginal, (__int64)1);
}

double GetCrossDistance(double x1, double y1, double x2, double y2)
{
	return CRT::m_sqrtf(CRT::m_powf((float)(x1 - x2), (float)(2)) + CRT::m_powf((float)(y1 - y2), (float)2));
}

PVOID GetClosestPlayerToCrossHair(Vector3 playerPos, float& maxDistance, PVOID entity)
{
	if (entity)
	{
		float crossDistance = GetCrossDistance(playerPos.x, playerPos.y, (crossX), (crossY));

		if (crossDistance < maxDistance)
		{
			float Radius = (AimBotFOV * crossX / cameraFov) / 2;

			if (playerPos.x <= ((crossX)+Radius) &&
				playerPos.x >= ((crossX)-Radius) &&
				playerPos.y <= ((crossY)+Radius) &&
				playerPos.y >= ((crossY)-Radius))
			{
				maxDistance = crossDistance;

				return entity;
			}

			return nullptr;
		}
	}

	return nullptr;
}

Matrix4 ToMatrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = CRT::m_sinf(radPitch);
	float CP = CRT::m_cosf(radPitch);
	float SY = CRT::m_sinf(radYaw);
	float CY = CRT::m_cosf(radYaw);
	float SR = CRT::m_sinf(radRoll);
	float CR = CRT::m_cosf(radRoll);

	Matrix4 matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

Vector3 WorldToScreenWithRotation(Vector3 WorldLocation, Vector3 CameraRotation)
{
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Rotation = CameraRotation;

	auto tempMatrix = ToMatrix(Rotation);

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - cameraLocation;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	float FovAngle = cameraFov;
	float ScreenCenterX = crossX;
	float ScreenCenterY = crossY;

	if (vTransformed.z < 1.f)
		return Vector3(0, 0, 0);

	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / CRT::m_tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / CRT::m_tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return Screenlocation;
}

PVOID GetTargetByFOV(PVOID playerController, PVOID entity, float& max)
{
	if (!entity)
		return nullptr;

	auto currentMesh = ReadPTR(entity, UEOffsets::Engine::Character::Mesh);

	if (!currentMesh)
		return nullptr;

	Vector3 headPos3D = UEUtils::GetBonePosition(currentMesh, UEBone::HEAD);

	if (headPos3D.x == 0 && headPos3D.y == 0)
		return nullptr;

	Vector3 headPos2D = WorldToScreenWithRotation(headPos3D, cameraRotation);

	if (headPos2D.x <= 0 && headPos2D.y <= 0)
		return nullptr;

	if (!UEUtils::IsVisible(playerController, entity) && !AimOnVisibles)
		return nullptr;

	auto target = GetClosestPlayerToCrossHair(headPos2D, max, entity);

	if (!target)
		return nullptr;

	return target;
}

Vector3 CalcAngles(PVOID currentMesh, int boneId)
{
	Vector3 angles = Vector3();

	if (!currentMesh)
		return angles;

	Vector3 BonePos = UEUtils::GetBonePosition(currentMesh, boneId);

	if (BonePos.x == 0 && BonePos.y == 0)
		return angles;

	Vector3 diff = BonePos - cameraLocation;

	float distance = diff.Length();

	angles.x = -(((float)CRT::m_acosf(diff.z / distance) * (float)(180.0f / M_PI)) - 90.f);
	angles.y = (float)CRT::m_atan2f(diff.y, diff.x) * (float)(180.0f / M_PI);

	return angles;
}

Vector3 inline SmoothAngles(Vector3 StartAngle, Vector3 EndAngle)
{
	Vector3 SmoothAngle = Vector3();

	SmoothAngle.x = (EndAngle.x - StartAngle.x) / AimBotSmooth + StartAngle.x;
	SmoothAngle.y = (EndAngle.y - StartAngle.y) / AimBotSmooth + StartAngle.y;

	return SmoothAngle;
}

VOID FaceEntity(PVOID PlayerController, PVOID CurrentActor, PVOID CurrentMesh, Vector3 LocalLocation, int BoneId)
{
	Vector3 targetAngles = Vector3();

	targetAngles = CalcAngles(CurrentMesh, BoneId);

	if (targetAngles.x == 0 && targetAngles.y == 0)
		return;

	targetAngles.z = 0;

	if (AimBotSmooth > 0)
		targetAngles = SmoothAngles(cameraRotation, targetAngles);

	UEUtils::SetControlRotation(PlayerController, targetAngles);
}

__forceinline VOID DrawHealthBar(Vector2 HeadPos, Vector2 FootPos, WORD Health, WORD MaxHealth)
{
	auto BarHeight = FootPos.y - HeadPos.y;
	auto BarWidth = (BarHeight / 2) * 1.25f;

	int BarX = (int)HeadPos.x - (BarWidth / 2);
	int BarY = (int)HeadPos.y;

	auto Percentage = Health * (BarHeight / MaxHealth);
	auto Deduction = BarHeight - Percentage;

	auto HealthColor = IM_COL32(0.0f, 255.0f, 0.0f, 255.0f);

	if (Health > 75)
		HealthColor = IM_COL32(0.0f, 255.0f, 0.0f, 255.0f);
	else if (Health > 40)
		HealthColor = IM_COL32(255.0f, 155.0f, 0.0f, 255.0f);
	else
		HealthColor = IM_COL32(255.0f, 0.0f, 0.0f, 255.0f);

	DrawBorderBox(BarX - 6, BarY, 3, (int)BarHeight, 1.5, IM_COL32(0.0f, 0.0f, 0.0f, 70.0f));
	DrawFilledRect(BarX - 6, BarY + Deduction, 3, Percentage, HealthColor);
}

VOID OnRender(ImGuiWindow* ImWindow)
{
	auto PlayerClass = STATIC_Soldier();

	crossX = (float)screenWidth / 2;
	crossY = (float)screenHeight / 2;

	float maxFov = 999.0f;

	auto World = *UEOffsets::uWorld;
	if (!World)
		return;

	auto GameInstance = ReadPTR(World, UEOffsets::Engine::World::OwningGameInstance);
	if (!GameInstance)
		return;

	auto LocalPlayers = ReadPTR(GameInstance, UEOffsets::Engine::GameInstance::LocalPlayers);
	if (!LocalPlayers)
		return;

	auto LocalPlayer = ReadPTR(LocalPlayers, 0);
	if (!LocalPlayer)
		return;

	auto LocalPlayerController = ReadPTR(LocalPlayer, UEOffsets::Engine::Player::PlayerController);
	if (!LocalPlayerController)
		return;

	auto LocalPawn = reinterpret_cast<UObject*>(ReadPTR(LocalPlayerController, UEOffsets::Engine::PlayerController::AcknowledgedPawn));
	if (!LocalPawn)
		return;

	auto LocalPlayerCameraManager = ReadPTR(LocalPlayerController, UEOffsets::Engine::PlayerController::PlayerCameraManager);
	if (!LocalPlayerCameraManager)
		return;

	if (UEUtils::IsDead(LocalPawn))
		return;

	if (!bSpeedHackInit && SpeedEnabled)
	{
		LPCSTR lpKernel32Name = xor ("Kernel32.dll");
		HMODULE hKernel32 = (HMODULE)ModuleUtils::GetModuleBase(lpKernel32Name);

		QueryPerformanceCounter_Original = (tQueryPerformanceCounter)ModuleUtils::GetFuncAddress(hKernel32, xor ("QueryPerformanceCounter"));

		LARGE_INTEGER liCounter;
		QueryPerformanceCounter_Original(&liCounter);

		g_SpeedPerfomance = SpeedPerfomance<LONGLONG>(liCounter.QuadPart, kInitialSpeed);

		CreateHook((uintptr_t)ModuleUtils::GetFuncAddress(hKernel32, xor ("QueryPerformanceCounter")), (uintptr_t)QueryPerformanceCounter_Hook, (long long*)&QueryPerformanceCounter_Original);

		bSpeedHackInit = true;
	}

	auto LocalCharacter = reinterpret_cast<UObject*>(ReadPTR(LocalPlayerController, 0x288));
	if (!LocalCharacter)
		return;

	if (bNoRecoil || bNoSpread || bNoSway)
	{
		auto CurrentWeapon = UEUtils::GetCurrentWeapon(LocalPawn);

		if (CurrentWeapon)
		{
			auto AnimScriptInstance = ReadPTR(CurrentWeapon, 0x440);

			if (AnimScriptInstance)
			{
				auto FireShake = ReadPTR(AnimScriptInstance, 0x928);

				if (FireShake)
				{
					if (bNoRecoil || bNoSpread)
					{
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9C0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9C4, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9C8, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9CC, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9D0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9D4, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9D8, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9DC, 0.0f);

						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9e0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9e4, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9e8, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0x9f4, 0.0f);

						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xa00, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xa0c, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xa18, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xa24, 0.0f);
					}

					if (bNoSway)
					{
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xac0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xac4, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xac8, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xacc, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xad0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xad4, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xaf0, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(AnimScriptInstance) + 0xaf4, 0.0f);
					}

					if (bNoSpread)
					{
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(FireShake) + 0x90, 0.0f);
						MemoryUtils::Write<float>(reinterpret_cast<uint64_t>(FireShake) + 0x148, 0.0f);
					}
				}
			}
		}
	}

	if (bSpeedHackInit)
	{
		if ((_GetAsyncKeyState(SpeedKey) & 0x8000))
		{
			SetSpeed(SpeedRatio);
		}
		else
		{
			SetSpeed(1.0);
		}
	}

	if (AimBotEnabled || DrawFOV)
		UpdateCameraInfo(LocalPlayerCameraManager);

	if (DrawFOV)
	{
		float fovRadius = (AimBotFOV * crossX / cameraFov) / 2;
		ImWindow->DrawList->AddCircle(ImVec2(screenWidth / 2, screenHeight / 2), fovRadius, IM_COL32(243, 243, 243, 255), 128);
	}

	if (!PlayerESP && !HealthBar && !NicknameESP && !DistanceESP && !AimBotEnabled)
		return;

	auto LocalPlayerLocation = UEUtils::GetPlayerPosition(LocalPawn);

	for (auto li = 0UL; li < ReadDWORD(World, UEOffsets::Engine::World::Levels + sizeof(PVOID)); ++li)
	{
		auto Levels = ReadPTR(World, UEOffsets::Engine::World::Levels);

		if (!Levels)
			break;

		auto Level = ReadPTR(Levels, li * sizeof(PVOID));

		if (!Level)
			continue;

		for (auto ai = 0UL; ai < ReadDWORD(Level, UEOffsets::Engine::Level::AActors + sizeof(PVOID)); ++ai)
		{
			auto Actors = ReadPTR(Level, UEOffsets::Engine::Level::AActors);
			if (!Actors)
				break;

			auto PlayerActor = reinterpret_cast<UObject*>(ReadPTR(Actors, ai * sizeof(PVOID)));
			if (!PlayerActor || PlayerActor == LocalPawn)
				continue;

			if (PlayerActor->IsA(PlayerClass))
			{
				if (UEUtils::IsDead(PlayerActor))
					continue;

				auto PlayerMesh = ReadPTR(PlayerActor, UEOffsets::Engine::Character::Mesh);

				if (!PlayerMesh)
					continue;

				auto PlayerState = ReadPTR(PlayerActor, UEOffsets::Engine::Pawn::PlayerState);

				if (!PlayerState)
					continue;

				if (UEUtils::IsInTeam(PlayerActor, LocalPawn))
					continue;

				Vector3 headPos3D = UEUtils::GetBonePosition(PlayerMesh, UEBone::HEAD);

				if (headPos3D.Invalid())
					continue;

				Vector3 rootPos3D = UEUtils::GetBonePosition(PlayerMesh, UEBone::ROOT);

				if (rootPos3D.Invalid())
					continue;

				headPos3D.z += 15;

				Vector2 headPos2D;
				if (UEUtils::WorldToScreen(LocalPlayerController, headPos3D, &headPos2D))
				{
					Vector2 bottomPos2D;
					if (UEUtils::WorldToScreen(LocalPlayerController, rootPos3D, &bottomPos2D))
					{
						if (headPos2D.Invalid())
							continue;

						if (bottomPos2D.Invalid())
							continue;

						auto PlayerLocation = UEUtils::GetPlayerPosition(PlayerActor);
						auto PlayerDistance = UEUtils::GetPlayerDistance(PlayerLocation, LocalPlayerLocation) / 100.0f;

						auto BoxColor = IM_COL32(243, 243, 243, 255);

						if (UEUtils::IsVisible(LocalPlayerController, PlayerActor))
							BoxColor = IM_COL32(70, 252, 137, 255);

						if (PlayerESP)
						{
							float CornerHeight = CRT::m_abs(headPos2D.y - bottomPos2D.y);
							float CornerWidth = CornerHeight * 0.65f;

							if (BoxStyle == 1)
							{
								DrawCornerESP(headPos2D.x - (CornerWidth / 2), headPos2D.y, CornerWidth, CornerHeight, BoxColor, 1.0f);
							}
							else
							{
								Draw2DBox(headPos2D.x - (CornerWidth / 2), headPos2D.y, CornerWidth, CornerHeight, 1, BoxColor);
							}
						}

						if (HealthBar)
						{
							float PlayerHealth = 0.0f;
							PlayerHealth = MemoryUtils::Read<float>(reinterpret_cast<uint64_t>(PlayerActor) + UEOffsets::Game::SQSoldier::Health);

							DrawHealthBar(headPos2D, bottomPos2D, PlayerHealth, 100.f);
						}

						if (NicknameESP)
						{
							auto playerName = UEUtils::GetPlayerName(PlayerState);

							if (playerName.IsValid())
							{
								wstring playerNameW = wstring(playerName.c_str());

								DrawOutlinedTextW(guiFont, playerNameW, ImVec2(headPos2D.x, headPos2D.y - 18), BoxColor, 14, true);

								UEUtils::Free((uintptr_t)playerName.c_str());
							}
						}

						if (DistanceESP)
						{
							string value_buf = to_string(PlayerDistance);

							string right_delim = xor (")");

							string distanceFormat = string(value_buf);
							distanceFormat = distanceFormat.substr(0, distanceFormat.length() - 7);

							string distanceInfo = xor ("(");
							distanceInfo.append(distanceFormat).append(right_delim);

							DrawOutlinedText(guiFont, distanceInfo, ImVec2(headPos2D.x, bottomPos2D.y + 6), BoxColor, 14, true);

							right_delim.clear();
							distanceFormat.clear();
							distanceInfo.clear();
						}

						if (AimBotEnabled)
						{
							if (_GetAsyncKeyState(AimBotKey) & 0x8000)
							{
								auto targetPlayer = GetTargetByFOV(LocalPlayerController, PlayerActor, maxFov);

								if (!targetPlayer)
									continue;

								auto TargetLocation = UEUtils::GetPlayerPosition(targetPlayer);
								auto TargetDistance = UEUtils::GetPlayerDistance(TargetLocation, LocalPlayerLocation) / 100.0f;

								if (TargetDistance > AimBotDistance)
									continue;

								auto targetMesh = ReadPTR(targetPlayer, UEOffsets::Engine::Character::Mesh);

								if (!targetMesh)
									continue;

								int32_t humanizedBoneId = GetAimBone();

								FaceEntity(LocalPlayerController, targetPlayer, targetMesh, LocalPlayerLocation, humanizedBoneId);
							}
						}
					}
				}
			}
		}
	}
}

HRESULT __fastcall PresentScene_Hook(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)
{
	if (!Device)
	{
		if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
		{
			SwapChain->GetDevice(__uuidof(Device), (void**)&Device);
			Device->GetImmediateContext(&DeviceContext);
		}

		ID3D11Texture2D* renderTargetTexture = nullptr;
		if (SUCCEEDED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&renderTargetTexture))))
		{
			Device->CreateRenderTargetView(renderTargetTexture, NULL, &RenderTargetView);
			renderTargetTexture->Release();
		}

		hwnd = LI_FN(FindWindowW).forwarded_safe_cached()(NULL, xor (L"SquadGame  "));

		if (!hwnd)
			Exit;

		ID3D11Texture2D* backBuffer = 0;
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);

		D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
		backBuffer->GetDesc(&backBufferDesc);

		screenWidth = (float)backBufferDesc.Width;
		screenHeight = (float)backBufferDesc.Height;

		auto imGuiContext = ImGui::CreateContext();

		ImGui::StyleColorsDark();

		bool hwndInit = ImGui_ImplWin32_Init(hwnd);
		bool d3dInit = ImGui_ImplDX11_Init(Device, DeviceContext);

		guiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(xor ("C:\\Windows\\Fonts\\Arial.ttf"), 16.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		bool deviceObjectsInit = ImGui_ImplDX11_CreateDeviceObjects();
	}

	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	if (!ImMenu)
	{
		auto ImWindow = BeginScene();

		OnRender(ImWindow);

		EndScene(ImWindow);
	}

	if (ImMenu)
	{
		OnDrawGui();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return PresentScene_Original(SwapChain, SyncInterval, Flags);
}

HRESULT __fastcall ResizeBuffers_Hook(IDXGISwapChain* SwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if (RenderTargetView)
		RenderTargetView->Release();

	if (DeviceContext)
		DeviceContext->Release();

	if (Device)
		Device->Release();

	Device = nullptr;

	return ResizeBuffers_Original(SwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

VOID WINAPI Squad_OnLoad(float Kek2, LPCSTR Kek, BYTE memes)
{
	if (!UEUtils::Init())
	{
		Exit;
	}

	if (!UEOffsets::Init())
	{
		Exit;
	}

	if (!ImGui_ImplWin32_InitData())
	{
		Exit;
	}

	if (!ImGui::InitHeapFunctions())
	{
		Exit;
	}

	if (!ImGui::InitApiFunctions())
	{
		Exit;
	}

	LPCSTR lpUser32Name = xor ("user32.dll");
	HMODULE hUser32 = (HMODULE)ModuleUtils::GetModuleBase(lpUser32Name);

	_GetWindowLongPtrA = (tGetWindowLongPtrA)ModuleUtils::GetFuncAddress(hUser32, xor ("GetWindowLongPtrA"));
	_SetWindowLongPtrA = (tSetWindowLongPtrA)ModuleUtils::GetFuncAddress(hUser32, xor ("SetWindowLongPtrA"));

	_GetAsyncKeyState = (tGetAsyncKeyState)ModuleUtils::GetFuncAddress(hUser32, xor ("GetAsyncKeyState"));

	_ClipCursor = (tClipCursor)ModuleUtils::GetFuncAddress(hUser32, xor ("ClipCursor"));

	_CallWindowProcA = (tCallWindowProcA)ModuleUtils::GetFuncAddress(hUser32, xor ("CallWindowProcA"));

	hwnd = LI_FN(FindWindowW).forwarded_safe_cached()(NULL, xor (L"SquadGame  "));

	if (!hwnd)
	{
		Exit;
	}

	WndProc_Original = (tWindowProc)_SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc_Hook);

	LPCSTR lpGameOverlay = xor ("GameOverlayRenderer64.dll");

	auto GameOverlayBase = ModuleUtils::GetModuleBase(lpGameOverlay);

	if (!GameOverlayBase)
		Exit;

	auto PresentScenePtr = ModuleUtils::PatternScanInModule(GameOverlayBase, xor ("4C 8D 05 ? ? ? ? 41 B9 ? ? ? ? 48 8D 15 ? ? ? ? E8 ? ? ? ? 48 8B 4F 50 48 8D 15 ? ? ? ? E8 ? ? ? ? 84 C0 75 1D 48 8B 4F 50 4C 8D 05 ? ? ? ? 41 B9 ? ? ? ? 48 8D 15 ? ? ? ? E8 ? ? ? ? 48 8B 4F 68 48 8D 15 ? ? ? ? E8 ? ? ? ? 84 C0 75 1D 48 8B 4F 68 4C 8D 05 ? ? ? ? 41 B9 ? ? ? ? 48 8D 15 ? ? ? ? E8 ? ? ? ?"));

	if (!PresentScenePtr)
	{
		Exit;
	}

	PresentScene_Original = (tPresentScene) * reinterpret_cast<uint64_t*>(ToRVA(PresentScenePtr, 7));

	if (!PresentScene_Original)
	{
		Exit;
	}

	*reinterpret_cast<uint64_t*>(ToRVA(PresentScenePtr, 7)) = reinterpret_cast<uint64_t>(PresentScene_Hook);

	auto ResizeBuffersPtr = ModuleUtils::PatternScanInModule(GameOverlayBase, xor ("4C 8D 05 ? ? ? ? 41 B9 ? ? ? ? 48 8D 15 ? ? ? ? E8 ? ? ? ? 48 8B 0B 4C 8D 44 24 ? 48 8D 15 ? ? ? ? 48 8B 01 FF 10"));

	if (!ResizeBuffersPtr)
	{
		Exit;
	}

	ResizeBuffers_Original = (tResizeBuffers) * reinterpret_cast<uint64_t*>(ToRVA(ResizeBuffersPtr, 7));

	if (!ResizeBuffers_Original)
	{
		Exit;
	}

	*reinterpret_cast<uint64_t*>(ToRVA(ResizeBuffersPtr, 7)) = reinterpret_cast<uint64_t>(ResizeBuffers_Hook);

	LI_FN(Beep).forwarded_safe_cached()(900, 200);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason)
{
	BYTE memes = 0xBD & 4 ^ 2 >> 5;
	
	if (!fdwReason)
		return TRUE;

	auto meme = xor ("plqwpgfpjmgmr2f2c");

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		ModuleUtils::EraseHeaders(hinstDLL);

		Squad_OnLoad(1.094f, meme, memes);

		return TRUE;
	}

	return TRUE;
}
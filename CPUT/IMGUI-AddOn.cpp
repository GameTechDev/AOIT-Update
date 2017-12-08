/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"


// DirectX
#include <d3d11.h>
#include <d3dcompiler.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#if 1
enum InputType
{
	DontCare,
	Mouse,
	Keyboard,
	Text,
};

// copypasted from imgui.cpp
static ImGuiWindow* FindHoveredWindow(ImVec2 pos, bool excluding_childs)
{
	ImGuiContext& g = *GImGui;
	for (int i = g.Windows.Size - 1; i >= 0; i--)
	{
		ImGuiWindow* window = g.Windows[i];
		if (!window->Active)
			continue;
		if (window->Flags & ImGuiWindowFlags_NoInputs)
			continue;
		if (excluding_childs && (window->Flags & ImGuiWindowFlags_ChildWindow) != 0)
			continue;

		// Using the clipped AABB so a child window will typically be clipped by its parent.
		ImRect bb;
		bb.Min.x = window->WindowRectClipped.Min.x - g.Style.TouchExtraPadding.x;
		bb.Min.y = window->WindowRectClipped.Min.y - g.Style.TouchExtraPadding.y;
		bb.Max.x = window->WindowRectClipped.Max.x + g.Style.TouchExtraPadding.x;
		bb.Max.y = window->WindowRectClipped.Max.y + g.Style.TouchExtraPadding.y;
		if (bb.Contains(pos))
			return window;
	}
	return NULL;
}

static ImGuiWindow* GetFrontMostModalRootWindow()
{
	ImGuiContext& g = *GImGui;
	for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
		if (ImGuiWindow* front_most_popup = g.OpenPopupStack.Data[n].Window)
			if (front_most_popup->Flags & ImGuiWindowFlags_Modal)
				return front_most_popup;
	return NULL;
}

// based on ImGui::NewFrame, but has no side effects
bool ImGuiWantCapture(InputType type)
{
	ImGuiContext& g = *GImGui;

	ImGuiWindow* hoveredWindow = FindHoveredWindow(g.IO.MousePos, false);
	int mouse_earliest_button_down = -1;
	bool mouse_any_down = false;
	for (int i = 0; i < IM_ARRAYSIZE(g.IO.MouseDown); i++)
	{
		bool mouseDown = (g.IO.MouseDownDuration[i] > 0.0f);
		mouse_any_down |= mouseDown;
		if (mouseDown)
			if (mouse_earliest_button_down == -1 || g.IO.MouseClickedTime[mouse_earliest_button_down] > g.IO.MouseClickedTime[i])
				mouse_earliest_button_down = i;
	}
	bool mouseDownOwned = false;
	if (mouse_earliest_button_down != -1)
	{
		mouseDownOwned = g.IO.MouseDownOwned[mouse_earliest_button_down];
		bool MouseClicked_earliest = g.IO.MouseDown[mouse_earliest_button_down] && (g.IO.MouseDownDuration[mouse_earliest_button_down] < 0.0f);
		if (MouseClicked_earliest)
		{
			mouseDownOwned = (hoveredWindow != NULL) || (!g.OpenPopupStack.empty());
		}
	}
	bool mouse_owned_by_application = mouse_earliest_button_down != -1 && !mouseDownOwned;

	bool WantCaptureMouse = false;
	if (g.CaptureMouseNextFrame != -1)
		WantCaptureMouse = (g.CaptureMouseNextFrame != 0);
	else
		WantCaptureMouse = (!mouse_owned_by_application && (g.HoveredWindow != NULL || mouse_any_down)) || (g.ActiveId != 0) || (!g.OpenPopupStack.empty());
	bool WantCaptureKeyboard = (g.CaptureKeyboardNextFrame != -1) ? (g.CaptureKeyboardNextFrame != 0) : (g.ActiveId != 0);


	bool WantTextInput = (g.ActiveId != 0 && g.InputTextState.Id == g.ActiveId);

	if (type == InputType::Mouse && WantCaptureMouse ||
		type == InputType::Keyboard && WantCaptureKeyboard ||
		type == InputType::Text && WantTextInput)
	{
		return true;
	}

	return false;
}

bool ImGui_ImplDX11_WndProcHandler2(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	auto io = &ImGui::GetIO();

	InputType type = InputType::DontCare;

	switch (message)
	{
	case WM_CREATE:
		io->KeyMap[ImGuiKey_Tab] = VK_TAB;
		io->KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io->KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io->KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io->KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io->KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io->KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io->KeyMap[ImGuiKey_Home] = VK_HOME;
		io->KeyMap[ImGuiKey_End] = VK_END;
		io->KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io->KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io->KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io->KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io->KeyMap[ImGuiKey_A] = 'A';
		io->KeyMap[ImGuiKey_C] = 'C';
		io->KeyMap[ImGuiKey_V] = 'V';
		io->KeyMap[ImGuiKey_X] = 'X';
		io->KeyMap[ImGuiKey_Y] = 'Y';
		io->KeyMap[ImGuiKey_Z] = 'Z';
		break;
	case WM_CHAR:
		if (wParam < 256)
		{
			io->AddInputCharacter((ImWchar)wParam);
		}
		type = InputType::Text;
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		int vkey = (int)wParam;
		io->KeysDown[vkey] = 1;
		if (vkey == VK_CONTROL) io->KeyCtrl = true;
		else if (vkey == VK_MENU) io->KeyAlt = true;
		else if (vkey == VK_SHIFT) io->KeyShift = true;
		type = InputType::Keyboard;
	}
	break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	{
		int vkey = (int)wParam;
		io->KeysDown[vkey] = 0;
		if (vkey == VK_CONTROL) io->KeyCtrl = false;
		else if (vkey == VK_MENU) io->KeyAlt = false;
		else if (vkey == VK_SHIFT) io->KeyShift = false;
	}
	break;
	case WM_MOUSEMOVE:
		io->MousePos.x = short(LOWORD(lParam));
		io->MousePos.y = short(HIWORD(lParam));
		type = InputType::Mouse;
		break;
	case WM_MOUSEWHEEL:
		io->MouseWheel = GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA);
		type = InputType::Mouse;
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (message == WM_LBUTTONDOWN) io->MouseDown[0] = 1;
		if (message == WM_RBUTTONDOWN) io->MouseDown[1] = 1;
		if (message == WM_MBUTTONDOWN) io->MouseDown[2] = 1;
		type = InputType::Mouse;
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (message == WM_LBUTTONUP) io->MouseDown[0] = 0;
		if (message == WM_RBUTTONUP) io->MouseDown[1] = 0;
		if (message == WM_MBUTTONUP) io->MouseDown[2] = 0;
		break;
	case WM_POINTERUPDATE:
	case WM_POINTERDOWN:
		type = InputType::Mouse;
		// Intentional Fallthrough:
		;
	case WM_POINTERUP:
	{
		WORD pointerId = GET_POINTERID_WPARAM(wParam);
		POINTER_INFO pointerInfo;

		if (::GetPointerInfo(pointerId, &pointerInfo))
		{
			if (pointerInfo.pointerType == PT_MOUSE)
			{
				// FIXME: handle other mouse buttons
				if (message == WM_POINTERDOWN)
				{
					io->MouseDown[0] = 1;
				}
				else if (message == WM_POINTERUP)
				{
					io->MouseDown[0] = 0;
				}

				POINT p = pointerInfo.ptPixelLocation;
				ScreenToClient(hWnd, &p);

				io->MousePos.x = (float)p.x;
				io->MousePos.y = (float)p.y;
			}
		}
	}
	break;
	}

	bool directInputToImgui = (type != InputType::DontCare) && ImGuiWantCapture(type);

	return directInputToImgui;
}

#endif
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"


BvKeyboard::KeyState g_CurrGlobalKeyStates[BvKeyboard::kMaxKeyCount]{};
BvKeyboard::KeyState g_PrevGlobalKeyStates[BvKeyboard::kMaxKeyCount]{};
u32 g_KeyStateChangeIndices[BvKeyboard::kMaxKeyCount]{};
u32 g_KeyStateChangeCount = 0;


void SetKeyStateFromKeyEvent(u32 vkCode, u32 scanCode, bool isKeyDown);
void SetKeyStateFromCharEvent(u32 vkCode, u32 codePoinnt, bool isDeadKey);


void UpdateInputBuffers()
{
	if (g_KeyStateChangeCount > 0)
	{
		for (auto i = 0u; i < g_KeyStateChangeCount; ++i)
		{
			u32 vkCode = g_KeyStateChangeIndices[i];
			g_PrevGlobalKeyStates[vkCode] = g_CurrGlobalKeyStates[vkCode];
		}
		g_KeyStateChangeCount = 0;
	}
}


BvKeyboard::BvKeyboard()
{
}


BvKeyboard::~BvKeyboard()
{
}


bool BvKeyboard::KeyWentDown(BvKey key) const
{
	return g_CurrGlobalKeyStates[u32(key)].m_IsPressed && !g_PrevGlobalKeyStates[u32(key)].m_IsPressed;
}


bool BvKeyboard::KeyWentUp(BvKey key) const
{
	return !g_CurrGlobalKeyStates[u32(key)].m_IsPressed && g_PrevGlobalKeyStates[u32(key)].m_IsPressed;
}


bool BvKeyboard::KeyIsDown(BvKey key) const
{
	return g_CurrGlobalKeyStates[u32(key)].m_IsPressed;
}


bool BvKeyboard::KeyIsUp(BvKey key) const
{
	return !g_CurrGlobalKeyStates[u32(key)].m_IsPressed;
}


const BvKeyboard::KeyState& BvKeyboard::GetKeyState(BvKey key) const
{
	return g_CurrGlobalKeyStates[u32(key)];
}


void ProcessLegacyKeyboardMessage(WPARAM wParam, LPARAM lParam, const BvKeyboard::KeyState*& pKeyState)
{
	pKeyState = nullptr;

	u32 vkCode = wParam;
	const bool isKeyUp = (HIWORD(lParam) & KF_UP) ? true : false;
	//isKeyRepeat = (lParam & 0x40000000) != 0;

	u32 scanCode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
	if (!scanCode)
	{
		// NOTE: Some synthetic key messages have a scancode of zero
		// HACK: Map the virtual key back to a usable scancode
		scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
	}

	// HACK: Alt+PrtSc has a different scancode than just PrtSc
	if (scanCode == 0x54)
		scanCode = 0x137;

	// HACK: Ctrl+Pause has a different scancode than just Pause
	if (scanCode == 0x146)
		scanCode = 0x45;

	// HACK: CJK IME sets the extended bit for right Shift
	if (scanCode == 0x136)
		scanCode = 0x36;

	// The Ctrl keys require special handling
	if (wParam == VK_CONTROL)
	{
		vkCode = (HIWORD(lParam) & KF_EXTENDED) ? VK_RCONTROL: VK_LCONTROL;
		if (vkCode == VK_LCONTROL)
		{
			// NOTE: Alt Gr sends Left Ctrl followed by Right Alt
			// HACK: We only want one event for Alt Gr, so if we detect
			//       this sequence we discard this Left Ctrl message now
			//       and later report Right Alt normally
			MSG next;
			const DWORD time = GetMessageTime();

			if (PeekMessage(&next, NULL, 0, 0, PM_NOREMOVE))
			{
				if (next.message == WM_KEYDOWN ||
					next.message == WM_SYSKEYDOWN ||
					next.message == WM_KEYUP ||
					next.message == WM_SYSKEYUP)
				{
					if (next.wParam == VK_MENU &&
						(HIWORD(next.lParam) & KF_EXTENDED) &&
						next.time == time)
					{
						// Next message is Right Alt down so discard this
						return;
					}
				}
			}
		}
	}
	else if (wParam == VK_MENU)
	{
		vkCode = (HIWORD(lParam) & KF_EXTENDED) ? VK_RMENU : VK_LMENU;
	}
	else if (wParam == VK_SHIFT)
	{
		vkCode = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);

		if (isKeyUp)
		{
			// HACK: Release both Shift keys on Shift up event, as when both
			// are pressed the first release does not emit any event
			if (g_PrevGlobalKeyStates[VK_LSHIFT].m_IsPressed && g_PrevGlobalKeyStates[VK_RSHIFT].m_IsPressed)
			{
				auto otherVkCode = vkCode == VK_RSHIFT ? VK_LSHIFT : VK_RSHIFT;
				SetKeyStateFromKeyEvent(otherVkCode, MapVirtualKey(otherVkCode, MAPVK_VK_TO_VSC_EX), !isKeyUp);
			}
		}
	}

	if (vkCode < BvKeyboard::kMaxKeyCount)
	{
		SetKeyStateFromKeyEvent(vkCode, scanCode, !isKeyUp);
		pKeyState = &g_CurrGlobalKeyStates[vkCode];
	}
}


void ProcessRawInputKeyboardMessage(const RAWKEYBOARD& rawKB, const BvKeyboard::KeyState*& pKeyState)
{
	pKeyState = nullptr;

	u32 vkCode = rawKB.VKey;
	u32 scanCode = rawKB.MakeCode;
	u32 flags = rawKB.Flags;

	if (vkCode == 255)
	{
		// discard "fake keys" which are part of an escaped sequence
		return;
	}
	else if (vkCode == VK_SHIFT)
	{
		// correct left-hand / right-hand SHIFT
		vkCode = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);

		// When holding shift while pressing something on the numpad with num lock on,
		// it'll report a shift press that won't map to either VK_LSHIFT or VK_RSHIFT,
		// so discard this fake press
		if (vkCode != VK_LSHIFT && vkCode != VK_RSHIFT)
		{
			return;
		}
	}
	else if (vkCode == VK_NUMLOCK)
	{
		// correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
		scanCode = (MapVirtualKey(vkCode, MAPVK_VK_TO_VSC) | 0x100);
	}

	// e0 and e1 are escape sequences used for certain special keys, such as PRINT and PAUSE/BREAK.
	// see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
	const bool isE0 = ((flags & RI_KEY_E0) != 0);
	const bool isE1 = ((flags & RI_KEY_E1) != 0);

	if (isE1)
	{
		// for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
		// however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
		if (vkCode == VK_PAUSE)
			scanCode = 0x45;
		else
			scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
	}

	switch (vkCode)
	{
	// right-hand CONTROL and ALT have their e0 bit set
	case VK_CONTROL: vkCode = isE0 ? VK_RCONTROL : VK_LCONTROL; break;
	case VK_MENU: vkCode = isE0 ? VK_RMENU : VK_LMENU; break;
	// NUMPAD ENTER has its e0 bit set
	case VK_RETURN: if (isE0) vkCode = VK_SEPARATOR; break;
	// the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
	// corresponding keys on the NUMPAD will not.
	case VK_INSERT: if (!isE0) vkCode = VK_NUMPAD0; break;
	case VK_DELETE: if (!isE0) vkCode = VK_DECIMAL; break;
	case VK_HOME: if (!isE0) vkCode = VK_NUMPAD7; break;
	case VK_END: if (!isE0) vkCode = VK_NUMPAD1; break;
	case VK_PRIOR: if (!isE0) vkCode = VK_NUMPAD9; break;
	case VK_NEXT: if (!isE0) vkCode = VK_NUMPAD3; break;
	// the standard arrow keys will always have their e0 bit set, but the
	// corresponding keys on the NUMPAD will not.
	case VK_LEFT: if (!isE0) vkCode = VK_NUMPAD4; break;
	case VK_RIGHT: if (!isE0) vkCode = VK_NUMPAD6; break;
	case VK_UP: if (!isE0) vkCode = VK_NUMPAD8; break;
	case VK_DOWN: if (!isE0) vkCode = VK_NUMPAD2; break;
	// NUMPAD 5 doesn't have its e0 bit set
	case VK_CLEAR: if (!isE0) vkCode = VK_NUMPAD5; break;
	}

	// a key can either produce a "make" or "break" scancode. this is used to differentiate between down-presses and releases
	// see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
	const bool isKeyUp = ((flags & RI_KEY_BREAK) != 0);

	// getting a human-readable string
	//u32 key = (scanCode << 16) | (isE0 << 24);
	//char buffer[512] = {};
	//GetKeyNameText((LONG)key, buffer, 512);

	if (vkCode < BvKeyboard::kMaxKeyCount)
	{
		SetKeyStateFromKeyEvent(vkCode, scanCode, !isKeyUp);
		pKeyState = &g_CurrGlobalKeyStates[vkCode];
	}
}


void ProcessCharInputMessage(u32 codePoint, LPARAM lParam, bool isDeadKey, const BvKeyboard::KeyState*& pKeyState)
{
	pKeyState = nullptr;

	u32 scanCode = (u32)((lParam >> 16) & 0xFF);
	u32 vkCode = MapVirtualKeyEx(scanCode, MAPVK_VSC_TO_VK_EX, nullptr);
	if (vkCode != 0)
	{
		SetKeyStateFromCharEvent(vkCode, codePoint, isDeadKey);
		pKeyState = &g_CurrGlobalKeyStates[vkCode];
	}
}


void SetKeyStateFromKeyEvent(u32 vkCode, u32 scanCode, bool isKeyDown)
{
	auto& keyState = g_CurrGlobalKeyStates[vkCode];
	keyState.m_Key = BvKey(vkCode);
	keyState.m_IsPressed = isKeyDown;
	if (isKeyDown)
	{
		keyState.m_ScanCode = scanCode;
	}
	else
	{
		keyState.m_CodePoint = 0;
		keyState.m_IsDeadKey = false;
	}

	g_KeyStateChangeIndices[g_KeyStateChangeCount++] = vkCode;
}


void SetKeyStateFromCharEvent(u32 vkCode, u32 codePoinnt, bool isDeadKey)
{
	auto& keyState = g_CurrGlobalKeyStates[vkCode];
	keyState.m_CodePoint = codePoinnt;
	keyState.m_IsDeadKey = isDeadKey;
}
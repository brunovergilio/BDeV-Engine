#include "BDeV/System/HID/BvKeyboard.h"
#include "BDeV/System/Debug/BvDebug.h"


constexpr u32 kKeyboardArraySize = 256 / (sizeof(u64) * CHAR_BIT);
constexpr u64 kU64Mask = 63;
constexpr u32 kDivShift = 6;


u64 g_CurrGlobalKeyStates[kKeyboardArraySize]{};
u64 g_PrevGlobalKeyStates[kKeyboardArraySize]{};


BvKeyboard::BvKeyboard()
{
}


BvKeyboard::~BvKeyboard()
{
}


bool BvKeyboard::KeyWentDown(BvKey key) const
{
	return (g_CurrGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) != 0
		&& (g_PrevGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) == 0;
}


bool BvKeyboard::KeyWentUp(BvKey key) const
{
	return (g_CurrGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) == 0
		&& (g_PrevGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) != 0;
}


bool BvKeyboard::KeyIsDown(BvKey key) const
{
	return (g_CurrGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) != 0;
}


bool BvKeyboard::KeyIsUp(BvKey key) const
{
	return (g_CurrGlobalKeyStates[u64(key) >> kDivShift] & (1ull << u64(u64(key) & kU64Mask))) == 0;
}


void BvKeyboard::ProcessRawInputKeyboardMessage(const RAWKEYBOARD& rawKB) const
{
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

	SetGlobalKeyboardStateData(BvKey(vkCode), scanCode, !isKeyUp);
}


void BvKeyboard::SetGlobalKeyboardStateData(BvKey key, u32 scanCode, bool isKeyDown) const
{
	auto index = u32(key) >> kDivShift;
	u64 mask = (1ull << u64(u64(key) & kU64Mask));

	bool keyWasAlreadyDown = isKeyDown && (g_PrevGlobalKeyStates[index] & mask) != 0;
	g_PrevGlobalKeyStates[index] = g_CurrGlobalKeyStates[index];
	if (isKeyDown)
	{
		g_CurrGlobalKeyStates[index] |= mask;
		if (!keyWasAlreadyDown)
		{
			OnKeyWentDown(key, scanCode);
		}
		OnKeyDown(key, scanCode);
	}
	else
	{
		g_CurrGlobalKeyStates[index] &= ~mask;
		OnKeyWentUp(key, scanCode);
	}
}


BvKeyboard* Input::GetKeyboard()
{
	static BvKeyboard keyboard;
	return &keyboard;
}
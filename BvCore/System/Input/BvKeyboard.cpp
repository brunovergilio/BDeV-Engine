#include "BvCore/System/Input/BvKeyboard.h"
#include "BvCore/System/Window/BvNativeWindow.h"


BvKeyboard::BvKeyboard()
{
}


BvKeyboard::~BvKeyboard()
{
}


void BvKeyboard::Link(BvNativeWindow& window)
{
	window.AddKeyboardCallback(this, [this](u32 vkCode, u32 scanCode, bool isKeyDown)
	{
		auto index = vkCode >> 3;
		u64 mask = (1ull << u64(vkCode & 7));

		if (isKeyDown)
		{
			m_KeyStates[index].fetch_or(mask, std::memory_order::memory_order_relaxed);
		}
		else
		{
			m_KeyStates[index].fetch_and(~mask, std::memory_order::memory_order_relaxed);
		}
	});
}


void BvKeyboard::Update()
{
	for (auto i = 0u; i < kKeyGroupCount; i++)
	{
		m_PrevKeyStates[i] = m_CurrKeyStates[i];
		m_CurrKeyStates[i] = m_KeyStates[i].load(std::memory_order::memory_order_relaxed);
		m_CurrKeyStates[i] = m_KeyStates[i].load(std::memory_order::memory_order_relaxed);
	}
}


bool BvKeyboard::IsKeyPressed(BvKey key) const
{
	auto index = u64(key) >> 3;
	u64 mask = (1ull << u64(u64(key) & 7));

	return (m_CurrKeyStates[index] & mask) != 0
		&& (m_PrevKeyStates[index] & mask) == 0;
}


bool BvKeyboard::IsKeyDown(BvKey key) const
{
	return (m_CurrKeyStates[u64(key) >> 3] & (1ull << u64(u64(key) & 7))) != 0;
}


bool BvKeyboard::IsKeyUp(BvKey key) const
{
	return (m_CurrKeyStates[u64(key) >> 3] & (1ull << u64(u64(key) & 7))) == 0;
}
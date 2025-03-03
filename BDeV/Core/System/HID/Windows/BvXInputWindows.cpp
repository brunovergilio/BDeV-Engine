#include "BDeV/Core/System/HID/BvXInput.h"
#include <Windows.h>
#include <Xinput.h>
#include <cmath>
#include <algorithm>


#pragma comment(lib, "xinput.lib")


constexpr u32 kMaxControllers = XUSER_MAX_COUNT;
struct ControllerData
{
	BvXInput::Controller::State m_CurrState{};
	BvXInput::Controller::State m_PrevState{};
	u32 m_PacketNumber = kU32Max;
	bool m_Enabled = false;
};
ControllerData g_Controllers[kMaxControllers];


class BvXInputHelper
{
public:
	BvXInputHelper()
	{
		for (auto i = 0u; i < kMaxControllers; ++i)
		{
			m_Controllers[i].m_Index = i;
		}
	}

	~BvXInputHelper() {}

	BV_INLINE BvXInput::Controller* GetController(u32 index) { return m_Controllers + index; }
	
private:
	BvXInput::Controller m_Controllers[kMaxControllers];
};


BvXInput::Controller* GetXInputController(u32 index)
{
	static BvXInputHelper helper;
	return helper.GetController(index);
}


static_assert(sizeof(BvXInput::Controller::State) == sizeof(XINPUT_GAMEPAD), "XInput controller state's size doesn't match XINPUT_GAMEPAD's size");


bool UpdateXInputController(u32 index)
{
	XINPUT_STATE state{};
	auto result = XInputGetState(index, &state);
	auto& controller = g_Controllers[index];
	bool isConnected = true;
	if (result == ERROR_SUCCESS)
	{
		controller.m_Enabled = true;
		controller.m_PrevState = controller.m_CurrState;
		if (state.dwPacketNumber != controller.m_PacketNumber)
		{
			controller.m_PacketNumber = state.dwPacketNumber;
			memcpy(&controller.m_CurrState, &state.Gamepad, sizeof(XINPUT_GAMEPAD));
		}
	}
	else
	{
		controller.m_Enabled = false;
		controller.m_CurrState = {};
		controller.m_PrevState = {};
		controller.m_PacketNumber = kU32Max;
		isConnected = false;
	}

	return isConnected;
}


void UpdateXInput()
{
	for (auto i = 0u; i < kMaxControllers; ++i)
	{
		auto& controller = g_Controllers[i];
		if (!controller.m_Enabled)
		{
			continue;
		}

		UpdateXInputController(i);
	}
}


bool BvXInput::Controller::KeyWentDown(BvXInputKey key) const
{
	auto& controller = g_Controllers[m_Index];
	return EHasFlag(controller.m_CurrState.m_KeyStates, key) && !EHasFlag(controller.m_PrevState.m_KeyStates, key);
}


bool BvXInput::Controller::KeyWentUp(BvXInputKey key) const
{
	auto& controller = g_Controllers[m_Index];
	return !EHasFlag(controller.m_CurrState.m_KeyStates, key) && EHasFlag(controller.m_PrevState.m_KeyStates, key);
}


bool BvXInput::Controller::KeyIsDown(BvXInputKey key) const
{
	auto& controller = g_Controllers[m_Index];
	return EHasFlag(controller.m_CurrState.m_KeyStates, key);
}


bool BvXInput::Controller::KeyIsUp(BvXInputKey key) const
{
	auto& controller = g_Controllers[m_Index];
	return !EHasFlag(controller.m_CurrState.m_KeyStates, key);
}


constexpr f32 k1Div32767 = 1.0f / 32767.0f;
constexpr f32 k1Div255 = 1.0f / 255.0f;


std::pair<f32, f32> BvXInput::Controller::GetLThumbValuesRadial(f32 deadZone) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normX = controller.m_CurrState.m_ThumbLX * k1Div32767;
	f32 normY = controller.m_CurrState.m_ThumbLY * k1Div32767;

	f32 magnitude = std::sqrtf(normX * normX + normY * normY);

	if (magnitude < deadZone)
	{
		return { 0.0f, 0.0f };  // Ignore small movements
	}

	// Normalize and scale to [0,1]
	f32 scale = std::min((magnitude - deadZone) / (1.0f - deadZone), 1.0f);

	return { normX / magnitude * scale, normY / magnitude * scale };
}


std::pair<f32, f32> BvXInput::Controller::GetRThumbValuesRadial(f32 deadZone) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normX = controller.m_CurrState.m_ThumbRX * k1Div32767;
	f32 normY = controller.m_CurrState.m_ThumbRY * k1Div32767;

	f32 magnitude = std::sqrtf(normX * normX + normY * normY);

	if (magnitude < deadZone)
	{
		return { 0.0f, 0.0f };  // Ignore small movements
	}

	// Normalize and scale to [0,1]
	f32 scale = std::min((magnitude - deadZone) / (1.0f - deadZone), 1.0f);

	return { normX / magnitude * scale, normY / magnitude * scale };
}


std::pair<f32, f32> BvXInput::Controller::GetLThumbValuesAxial(f32 deadZoneX, f32 deadZoneY) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normX = controller.m_CurrState.m_ThumbLX * k1Div32767;
	f32 normY = controller.m_CurrState.m_ThumbLY * k1Div32767;

	// Apply dead zones per axis independently
	normX = (std::abs(normX) < deadZoneX) ? 0.0f : (normX - std::copysign(deadZoneX, normX)) / (1.0f - deadZoneX);
	normY = (std::abs(normY) < deadZoneY) ? 0.0f : (normY - std::copysign(deadZoneY, normY)) / (1.0f - deadZoneY);

	// Clamp to [-1,1] range
	return { std::clamp(normX, -1.0f, 1.0f), std::clamp(normY, -1.0f, 1.0f) };
}


std::pair<f32, f32> BvXInput::Controller::GetRThumbValuesAxial(f32 deadZoneX, f32 deadZoneY) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normX = controller.m_CurrState.m_ThumbRX * k1Div32767;
	f32 normY = controller.m_CurrState.m_ThumbRY * k1Div32767;

	// Apply dead zones per axis independently
	normX = (std::abs(normX) < deadZoneX) ? 0.0f : (normX - std::copysign(deadZoneX, normX)) / (1.0f - deadZoneX);
	normY = (std::abs(normY) < deadZoneY) ? 0.0f : (normY - std::copysign(deadZoneY, normY)) / (1.0f - deadZoneY);

	// Clamp to [-1,1] range
	return { std::clamp(normX, -1.0f, 1.0f), std::clamp(normY, -1.0f, 1.0f) };
}


f32 BvXInput::Controller::GetLTriggerValue(f32 deadZone) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normTrigger = controller.m_CurrState.m_LeftTrigger * k1Div255;
	if (normTrigger < deadZone)
	{
		return 0.0f; // Ignore small trigger presses
	}

	return (normTrigger - deadZone) / (1.0f - deadZone);
}


f32 BvXInput::Controller::GetRTriggerValue(f32 deadZone) const
{
	auto& controller = g_Controllers[m_Index];
	f32 normTrigger = controller.m_CurrState.m_RightTrigger * k1Div255;
	if (normTrigger < deadZone)
	{
		return 0.0f; // Ignore small trigger presses
	}

	return (normTrigger - deadZone) / (1.0f - deadZone);
}


const BvXInput::Controller::State& BvXInput::Controller::GetState() const
{
	return g_Controllers[m_Index].m_CurrState;
}


void BvXInput::Controller::SetVibration(u16 leftMotorSpeed, u16 rightMotorSpeed) const
{
	XINPUT_VIBRATION vibration{ leftMotorSpeed, rightMotorSpeed };
	XInputSetState(m_Index, &vibration);
}


bool BvXInput::Controller::UpdateControllerStatus() const
{
	return UpdateXInputController(m_Index);
}


BvXInput::Controller* BvXInput::GetController(u32 index) const
{
	return GetXInputController(index);
}
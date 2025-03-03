#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include <utility>


class BvXInput final
{
	BV_NOCOPYMOVE(BvXInput);

public:
	class Controller
	{
		BV_NOCOPYMOVE(Controller);

		Controller() = default;
		~Controller() = default;

	public:
		friend class BvXInputHelper;

		static constexpr f32 kDefaultDeadZoneLThumb = 0.239539780877102f;
		static constexpr f32 kDefaultDeadZoneRThumb = 0.2651753288369396f;
		static constexpr f32 kDefaultDeadZoneTrigger = 0.1176470588235294f;

		struct State
		{
			BvXInputKey m_KeyStates;
			u8 m_LeftTrigger;
			u8 m_RightTrigger;
			i16 m_ThumbLX;
			i16 m_ThumbLY;
			i16 m_ThumbRX;
			i16 m_ThumbRY;
		};

		// Checks if the key was just pressed
		bool KeyWentDown(BvXInputKey key) const;

		// Checks if the key was just released
		bool KeyWentUp(BvXInputKey key) const;

		// Checks if the key is down
		bool KeyIsDown(BvXInputKey key) const;

		// Checks if the key is up
		bool KeyIsUp(BvXInputKey key) const;

		// Get thumb values using Radial Dead Zone
		std::pair<f32, f32> GetLThumbValuesRadial(f32 deadZone = kDefaultDeadZoneLThumb) const;
		std::pair<f32, f32> GetRThumbValuesRadial(f32 deadZone = kDefaultDeadZoneRThumb) const;

		// Get thumb values using Axial Dead Zone
		std::pair<f32, f32> GetLThumbValuesAxial(f32 deadZoneX = kDefaultDeadZoneLThumb, f32 deadZoneY = kDefaultDeadZoneLThumb) const;
		std::pair<f32, f32> GetRThumbValuesAxial(f32 deadZoneX = kDefaultDeadZoneRThumb, f32 deadZoneY = kDefaultDeadZoneRThumb) const;

		// Get trigger values using Trigger Dead Zone
		f32 GetLTriggerValue(f32 deadZone = kDefaultDeadZoneTrigger) const;
		f32 GetRTriggerValue(f32 deadZone = kDefaultDeadZoneTrigger) const;

		// Retrieves the controller's current state
		const State& GetState() const;

		// Sets vibration on the controller
		void SetVibration(u16 leftMotorSpeed, u16 rightMotorSpeed) const;

		// Checks if the controller is connected and update its status; Do
		// not call this every frame, as it may hinder performance
		bool UpdateControllerStatus() const;
		
	private:
		u32 m_Index = kU32Max;
	};

	BvXInput() = default;
	~BvXInput() = default;

	Controller* GetController(u32 index) const;
};
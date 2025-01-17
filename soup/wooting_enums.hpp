#pragma once

#include <cstdint>

#include "base.hpp"

NAMESPACE_SOUP
{
	enum class WootingCommand : uint8_t
	{
		Ping = 0,
		GetVersion = 1,
		ResetToBootloader = 2,
		GetSerial = 3,
		GetRgbProfileCount = 4,
		REMOVED_GetCurrentRgbProfileIndex = 5,
		REMOVED_GetRgbMainProfile = 6,
		ReloadProfile0 = 7,
		SaveRgbProfile = 8,
		GetDigitalProfilesCount = 9,
		GetAnalogProfilesCount = 10,
		GetCurrentKeyboardProfileIndex = 11,
		GetDigitalProfile = 12,
		GetAnalogProfileMainPart = 13,
		GetAnalogProfileCurveChangeMapPart1 = 14,
		GetAnalogProfileCurveChangeMapPart2 = 15,
		GetNumberOfKeys = 16,
		GetMainMappingProfile = 17,
		GetFunctionMappingProfile = 18,
		GetDeviceConfig = 19,
		GetAnalogValues = 20,
		KeysOff = 21,
		KeysOn = 22,
		ActivateProfile = 23,
		getDKSProfile = 24,
		doSoftReset = 25,
		REMOVED_GetRgbColorsPart1 = 26,
		REMOVED_GetRgbColorsPart2 = 27,
		REMOVED_GetRgbEffects = 28,
		RefreshRgbColors = 29,
		WootDevSingleColor = 30,
		WootDevResetColor = 31,
		WootDevResetAll = 32,
		WootDevInit = 33,
		REMOVED_GetRgbProfileBase = 34,
		GetRgbProfileColorsPart1 = 35,
		GetRgbProfileColorsPart2 = 36,
		REMOVED_GetRgbProfileEffect = 37,
		ReloadProfile = 38,
		GetKeyboardProfile = 39,
		GetGamepadMapping = 40,
		GetGamepadProfile = 41,
		SaveKeyboardProfile = 42,
		ResetSettings = 43,
		SetRawScanning = 44,
		StartXinputDetection = 45,
		StopXinputDetection = 46,
		SaveDKSProfile = 47,
		GetMappingProfile = 48,
		GetActuationProfile = 49,
		GetRgbProfileCore = 50,
		GetGlobalSettings = 51,
		GetAKCProfile = 52,
		SaveAKCProfile = 53,
		GetRapidTriggerProfile = 54,
		GetProfileMetadata = 55,
		IsFLashChipConnected = 56,
		GetRgbLayer = 57,
	};

	enum class WootingReport : uint8_t
	{
		REMOVED_RgbMainPart = 0,
		REMOVED_DigitalProfileMainPart = 1,
		REMOVED_AnalogProfileMainPart = 2,
		REMOVED_AnalogProfileCurveChangeMapPart1 = 3,
		REMOVED_AnalogProfileCurveChangeMapPart2 = 4,
		REMOVED_MainMappingProfile = 5,
		REMOVED_FunctionMappingProfile = 6,
		DeviceConfig = 7,
		SetDKSProfile = 8,
		RgbColorsPart = 9,
		REMOVED_RgbEffects = 10,
		WootDevRawReport = 11,
		SerialNumber = 12,
		REMOVED_RgbProfileBase = 13,
		RgbProfileColorsPart1 = 14,
		RgbProfileColorsPart2 = 15,
		REMOVED_RgbProfileEffect = 16,
		KeyboardProfile = 17,
		GamepadMapping = 18,
		GamepadProfile = 19,
		MappingProfile = 20,
		ActuationProfile = 21,
		RgbProfileCore = 22,
		GlobalSettings = 23,
		AKCProfile = 24,
		RapidTriggerProfile = 25,
		ProfileMetadata = 26,
		RgbLayer = 27,
	};
}

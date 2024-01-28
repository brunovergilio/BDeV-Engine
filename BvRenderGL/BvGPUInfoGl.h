#pragma once


struct BvGPUInfoGl
{
	static constexpr auto kMaxDeviceNameSize = 32u;
	static constexpr auto kMaxDriverNameSize = 64u;

	uint32_t majorVersion{};
	uint32_t minorVersion{};
	char driverName[kMaxDriverNameSize]{};
	char deviceName[kMaxDeviceNameSize]{};
	char vendorName[kMaxDeviceNameSize]{};
	char shaderVersionName[kMaxDeviceNameSize]{};
	
	struct
	{
		bool directStateAccess : 1;
		bool textureFilterAnisotropic : 1;
		bool polygonOffsetClamp : 1;
		bool nvConservativeRaster : 1;
		bool intelConservativeRaster : 1;
		bool depthBoundsTest : 1;
	} m_ExtendedFeatures{};
};
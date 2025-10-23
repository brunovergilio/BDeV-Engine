#pragma once


#include "BDeV/Core/System/BvPlatformHeaders.h"


namespace Internal
{

	class BvWinsockInitializer
	{
		BvWinsockInitializer();
		~BvWinsockInitializer();

	public:
		static const BvWinsockInitializer& GetInstance();
	};
}
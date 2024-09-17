#pragma once


#include "BDeV/Core/BvCore.h"


class BvMemorySystem final
{
public:
	BvMemorySystem();
	~BvMemorySystem();

	void Initialize();
	void Shutdown();
};
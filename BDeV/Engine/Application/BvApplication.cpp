#include "BvApplication.h"
#include <BDeV/System/Platform/BvPlatform.h>


void BvApplication::Run()
{
	Initialize();

	while (IsRunning())
	{
		ProcessFrame();
	}

	Shutdown();
}


void BvApplication::Initialize()
{
	BvPlatform::Initialize();

	m_IsRunning.store(true, std::memory_order::relaxed);
}


void BvApplication::Shutdown()
{
	BvPlatform::Shutdown();
}


void BvApplication::ProcessFrame()
{
	BvPlatform::ProcessOSEvents();

	PreUpdate();
	Update();
	PostUpdate();
}


void BvApplication::Stop()
{
	m_IsRunning.store(false, std::memory_order::relaxed);
}


bool BvApplication::IsRunning() const
{
	return m_IsRunning.load(std::memory_order::relaxed);
}
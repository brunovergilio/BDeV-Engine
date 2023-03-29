#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include <atomic>


class BV_API BvApplication
{
	BV_NOCOPYMOVE(BvApplication);

public:
	void Run();

protected:
	BvApplication() {}
	virtual ~BvApplication() {}

	virtual void Initialize();
	virtual void Shutdown();

	virtual void ProcessFrame();

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	void Stop();
	bool IsRunning() const;

protected:
	std::atomic<bool> m_IsRunning = false;
};
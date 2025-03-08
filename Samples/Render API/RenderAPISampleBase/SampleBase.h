#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/HID/BvMouse.h"
#include "UIOverlay.h"


class SampleBase
{
public:
	SampleBase();
	virtual ~SampleBase();

	virtual void Initialize();
	virtual void Update();
	virtual void UpdateUI() {}
	virtual void Draw() {}
	virtual void DrawUI() {}
	virtual void Shutdown();
	virtual bool IsDone() { return m_IsDone; }

protected:
	BvApplication m_App;
	BvWindow* m_pWindow = nullptr;
	UIOverlay m_Overlay;
	BvKeyboard m_Keyboard;
	BvMouse m_Mouse;
	f32 m_Dt;
	bool m_IsDone = false;
};


#define SAMPLE_MAIN(sampleObj)		\
int main(int argc, char* argv[])	\
{									\
	sampleObj sample;				\
	sample.Initialize();			\
	while (!sample.IsDone())		\
	{								\
		sample.Update();			\
		sample.UpdateUI();			\
		sample.Draw();				\
		sample.DrawUI();			\
	}								\
	sample.Shutdown();				\
									\
	return 0;						\
}
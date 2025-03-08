#include "SampleBase.h"


SampleBase::SampleBase()
{
}


SampleBase::~SampleBase()
{
}


void SampleBase::Initialize()
{
	m_App.Initialize();
	m_App.RegisterRawInput(true, true);
}


void SampleBase::Update()
{
	m_App.ProcessOSEvents();
}


void SampleBase::Shutdown()
{
	m_App.Shutdown();
}
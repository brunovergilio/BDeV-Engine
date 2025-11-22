#pragma once


#include "BvCommonGl.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvDebugReportGl
{
	BV_NOCOPYMOVE(BvDebugReportGl);

public:
	BvDebugReportGl();
	~BvDebugReportGl();

	void Create();
	void Destroy();

private:
	static void DebugMessageCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* pMsg, void const* pUserData);
};
#include "BvDebugReportGl.h"


BvDebugReportGl::BvDebugReportGl()
{
	Create();
}


BvDebugReportGl::~BvDebugReportGl()
{
	Destroy();
}


void BvDebugReportGl::Create()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(DebugMessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
}


void BvDebugReportGl::Destroy()
{
	glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDisable(GL_DEBUG_OUTPUT);
}


void BvDebugReportGl::DebugMessageCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* pMsg, void const* pUserData)
{
	// TODO: Added proper validation
	switch (src)
	{
	case GL_DEBUG_SOURCE_API:				break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		break;
	case GL_DEBUG_SOURCE_APPLICATION:		break;
	case GL_DEBUG_SOURCE_OTHER:				break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:				break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	break;
	case GL_DEBUG_TYPE_PORTABILITY:			break;
	case GL_DEBUG_TYPE_PERFORMANCE:			break;
	case GL_DEBUG_TYPE_MARKER:				break;
	case GL_DEBUG_TYPE_OTHER:				break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_NOTIFICATION:	break;
	case GL_DEBUG_SEVERITY_LOW:				break;
	case GL_DEBUG_SEVERITY_MEDIUM:			break;
	case GL_DEBUG_SEVERITY_HIGH:			break;
	}
}
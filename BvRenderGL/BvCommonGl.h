#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif

#include <GL/glew.h>

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <GL/wglew.h>
#endif
cmake_minimum_required(VERSION 3.20)

set(HEADER_FILES 
	BDeV.h
	Core/BvCore.h
	Core/Container/BvFixedVector.h
	Core/Container/BvIterator.h
	Core/Container/BvQueue.h
	Core/Container/BvRobinMap.h
	Core/Container/BvRobinSet.h
	Core/Container/BvString.h
	Core/Container/BvStringId.h
	Core/Container/BvVector.h
	Core/Math/BvGeometryGenerator.h
	Core/Math/BvIntersections.h
	Core/Math/BvMath.h
	Core/Math/BvPrimitives.h
	Core/Math/BvTrigo.h
	Core/Math/BvMathLoaders.inl
	Core/Math/BvMathMatrix.inl
	Core/Math/BvMathQuaternion.inl
	Core/Math/BvMathVector.inl
	Core/RenderAPI/BvAccelerationStructure.h
	Core/RenderAPI/BvBuffer.h
	Core/RenderAPI/BvBufferView.h
	Core/RenderAPI/BvColor.h
	Core/RenderAPI/BvCommandContext.h
	Core/RenderAPI/BvGPUFence.h
	Core/RenderAPI/BvPipelineState.h
	Core/RenderAPI/BvQuery.h
	Core/RenderAPI/BvRenderAPIUtils.h
	Core/RenderAPI/BvRenderCommon.h
	Core/RenderAPI/BvRenderDevice.h
	Core/RenderAPI/BvRenderEngine.h
	Core/RenderAPI/BvRenderPass.h
	Core/RenderAPI/BvSampler.h
	Core/RenderAPI/BvShader.h
	Core/RenderAPI/BvShaderBindingTable.h
	Core/RenderAPI/BvShaderCompiler.h
	Core/RenderAPI/BvShaderResource.h
	Core/RenderAPI/BvSwapChain.h
	Core/RenderAPI/BvTexture.h
	Core/RenderAPI/BvTextureLoader.h
	Core/RenderAPI/BvTextureView.h
	Core/System/BvPlatformHeaders.h
	Core/System/Application/BvApplication.h
	Core/System/Application/BvMessageHandler.h
	Core/System/Diagnostics/BvDiagnostics.h
	Core/System/File/BvAsyncFile.h
	Core/System/File/BvFile.h
	Core/System/File/BvFileCommon.h
	Core/System/File/BvFileSystem.h
	Core/System/File/BvMemoryFile.h
	Core/System/File/BvPath.h
	Core/System/HID/BvHIDCommon.h
	Core/System/HID/BvKeyboard.h
	Core/System/HID/BvMouse.h
	Core/System/HID/BvXInput.h
	Core/System/Library/BvSharedLib.h
	Core/System/Memory/BvMemory.h
	Core/System/Memory/BvMemoryArena.h
	Core/System/Memory/Allocators/BvDefaultAllocator.h
	Core/System/Memory/Allocators/BvHeapAllocator.h
	Core/System/Memory/Allocators/BvLinearAllocator.h
	Core/System/Memory/Allocators/BvPoolAllocator.h
	Core/System/Memory/Allocators/BvStackAllocator.h
	Core/System/Memory/Utilities/BvBoundsChecker.h
	Core/System/Memory/Utilities/BvMemoryMarker.h
	Core/System/Memory/Utilities/BvMemoryTracker.h
	Core/System/Process/BvProcess.h
	Core/System/Threading/BvFiber.h
	Core/System/Threading/BvSync.h
	Core/System/Threading/BvThread.h
	Core/System/Window/BvMonitor.h
	Core/System/Window/BvWindow.h
	Core/System/Window/BvWindowCommon.h
	Core/Utils/BvEvent.h
	Core/Utils/BvHash.h
	Core/Utils/BvObject.h
	Core/Utils/BvRandom.h
	Core/Utils/BvRTTI.h
	Core/Utils/BvTask.h
	Core/Utils/BvTestUnit.h
	Core/Utils/BvText.h
	Core/Utils/BvTime.h
	Core/Utils/BvUtils.h
	Core/Utils/BvUUID.h
	Engine/JobSystem/BvParallelJobSystem.h
)
	
set(SRC_FILES
	Core/Container/BvStringId.cpp
	Core/Math/BvGeometryGenerator.cpp
	Core/Math/BvIntersections.cpp
	Core/RenderAPI/BvColor.cpp
	Core/RenderAPI/BvRenderAPIUtils.cpp
	Core/System/Diagnostics/BvDiagnostics.cpp
	Core/System/File/BvMemoryFile.cpp
	Core/System/HID/BvHIDCommon.cpp
	Core/System/Memory/BvMemory.cpp
	Core/System/Memory/Allocators/BvDefaultAllocator.cpp
	Core/System/Memory/Allocators/BvHeapAllocator.cpp
	Core/System/Memory/Allocators/BvLinearAllocator.cpp
	Core/System/Memory/Allocators/BvPoolAllocator.cpp
	Core/System/Memory/Allocators/BvStackAllocator.cpp
	Core/System/Memory/Utilities/BvBoundsChecker.cpp
	Core/System/Memory/Utilities/BvMemoryMarker.cpp
	Core/System/Memory/Utilities/BvMemoryTracker.cpp
	Core/System/Threading/BvSync.cpp
	Core/Utils/BvHash.cpp
	Core/Utils/BvTestUnit.cpp
	Core/Utils/BvText.cpp
	Core/Utils/BvTime.cpp
	Core/Utils/BvUUID.cpp
	Engine/JobSystem/BvParallelJobSystem.cpp
)

if (WIN32)
	set(WINDOWS_HEADER_FILES
		Core/System/File/Windows/BvAsyncFileWindows.h
		Core/System/Window/Windows/BvWindowWindows.h
	)
	
	enable_language(ASM_MASM)
	
	set(WINDOWS_SRC_FILES
		Core/System/Application/Windows/BvApplicationWindows.cpp
		Core/System/Diagnostics/Windows/BvDiagnosticsWindows.cpp
		Core/System/File/Windows/BvAsyncFileWindows.cpp
		Core/System/File/Windows/BvFileSystemWindows.cpp
		Core/System/File/Windows/BvFileWindows.cpp
		Core/System/File/Windows/BvPathWindows.cpp
		Core/System/HID/Windows/BvKeyboardWindows.cpp
		Core/System/HID/Windows/BvMouseWindows.cpp
		Core/System/HID/Windows/BvXInputWindows.cpp
		Core/System/Library/Windows/BvSharedLibWindows.cpp
		Core/System/Memory/Windows/BvMemoryWindows.cpp
		Core/System/Process/Windows/BvProcessWindows.cpp
		Core/System/Threading/Windows/BvFiberWindows.cpp
		Core/System/Threading/Windows/BvThreadWindows.cpp
		Core/System/Threading/Windows/BvFiberMASM.asm
		Core/System/Window/Windows/BvMonitorWindows.cpp
		Core/System/Window/Windows/BvWindowWindows.cpp
	)
	
	set_source_files_properties(Core/System/Threading/Windows/BvFiberMASM.asm PROPERTIES LANGUAGE ASM_MASM)
	
    list(APPEND HEADER_FILES ${WINDOWS_HEADER_FILES})
    list(APPEND SRC_FILES ${WINDOWS_SRC_FILES})
endif()

set(SOURCE_LIST ${SRC_FILES} ${HEADER_FILES})

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

add_library(BDeV STATIC ${SRC_FILES} ${HEADER_FILES} BvCore.natvis)

# Add include directories
target_include_directories(BDeV PUBLIC
    ${CMAKE_SOURCE_DIR}   # So includes will work
)

# Add macros
target_compile_definitions(BDeV PRIVATE
    ${COMMON_COMPILE_DEFINITIONS}
    $<$<CONFIG:Debug>:${COMMON_DEBUG_DEFINITIONS}>
    $<$<CONFIG:Release>:${COMMON_RELEASE_DEFINITIONS}>
    $<$<CONFIG:ReleaseDbgInfo>:${COMMON_RELDBGINFO_DEFINITIONS}>
    $<$<CONFIG:ReleaseMinSize>:${COMMON_MINSIZEREL_DEFINITIONS}>
)

# Add compiler options
target_compile_options(BDeV PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${COMMON_COMPILE_OPTIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CONFIG:Debug>:${COMMON_DEBUG_OPTIONS}>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CONFIG:Release>:${COMMON_RELEASE_OPTIONS}>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CONFIG:ReleaseDbgInfo>:${COMMON_RELDBGINFO_OPTIONS}>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CONFIG:ReleaseMinSize>:${COMMON_MINSIZEREL_OPTIONS}>>
)

if(MSVC)
	# using target_link_options() doesn't seem to enable the "Whole Program Optimization" field
	# in the VC project settings, even though /LTCG will be enabled
    set_target_properties(BDeV PROPERTIES 
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
		VS_USE_DEBUG_LIBRARIES "$<CONFIG:Debug>"
    )
endif()

# Add linker options (only exe/dll files)
#target_link_options(BDeV PRIVATE
#    $<$<CONFIG:Release>:${COMMON_RELEASE_LINKER_OPTIONS}>
#)
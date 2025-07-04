cmake_minimum_required(VERSION 3.20)

# Solution name
project(BDeV-Engine)

# C++ 20 - make it a requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce 64-bit
set(CMAKE_GENERATOR_PLATFORM x64)

# Restrict configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseDbgInfo;ReleaseMinSize" CACHE STRING "" FORCE)

# Make a mapping between my custom names and cmake's default ones - this can help with building dependencies
set(CMAKE_MAP_IMPORTED_CONFIG_ReleaseDbgInfo Release RelWithDebInfo)
set(CMAKE_MAP_IMPORTED_CONFIG_ReleaseMinSize Release MinSizeRel)

# Set binary file paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>)

# Helps on Windows path length issues
set(CMAKE_OBJECT_PATH_MAX 260)

# Platforms supported
if (WIN32)
	if (NOT MSVC)
		message(FATAL_ERROR "Windows platform only supported through MSVC compiler")
	endif()
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

# Load common settings globally
include(CMakeSettings.cmake)

# Include main project
add_subdirectory(BDeV)
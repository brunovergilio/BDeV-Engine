# BDeV Engine
This is my own Engine / Toolset that I use for C++ and Graphics / Systems programming, written with C++20. As of right now it only runs on Windows with MSVC as the compiler, with plans to cover other platforms as Linux and MacOS in the future. Here's what these projects contains at the moment:

## BDeV
The main project - should only be compiled as a static library for the moment. Features:
- Custom containers
	- Vector
	- Fixed Vector
	- Robin Hood Hash Set / Map
	- String
	- String Id
	- Iterators
- Core Functionality
	- File System / Paths / File and Async File handling
	- DLLs
	- Memory (stack / heap / virtual) / Allocators / Utitilies (Memory Markers, Bounds Checkers and Memory Trackers)
	System information (Processor Info / Stack trace)
	Threads / Fibers / Custom Synchronization Objects
	Windowing System / HID handling
	Diagnostics (Debug, Error and Logging Functionality)
- Utilities (Hashing / PRNG / Test Unit, etc)
- FPU / SIMD Math library (vectors / matrices / quaternions)
- RenderAPI interface
	- Most features included (Graphics / Mesh / Compute / Ray Tracing pipelines, Render Passes, MSAA, etc)
	- Still missing sparse bindings and video processing
- Engine-level features
	- Parallel Job System (based on id Tech 4)

## Plugins
These are projects that can be compiled as both static and shared libraries. Some of them are work in progress or have been paused.
- **BvRenderVk**
My Vulkan-based render interface.
- **BvRenderGL**
Supposed to be my OpenGL render interface - for now it's been paused, but I plan on getting back to it, if for anything just learning.
- **BvRenderD3D12**
Direct3D 12 render interface - just started coding it, will work on it as time allows.
- **BvRenderTools**
Extra functionality for projects during development phase.
	- Shader compilation (GLSLang for now)
	- Texture Loaders (DDS, based on DirectXTex and STB for other formats)
- **BvReflector**
Project that was supposed to pre-generate reflection data. Paused for now.

There are some other projects that I left as I used them when I started (**CoreTests** / **RenderTest**) - those should be ignored.
Lastly, I've got some Render API samples going; they're nothing fancy, but mainly a proof-of-concept to guarantee all my functionality is actually working.
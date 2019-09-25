# BDeV-Engine

This is my own Engine, which I've remade about a thousand times at this point. Right now it only contains static libraries, but I intend to start working on demos for it, as well as unit tests as it develops further.

Here's what I've got so far:

```
* Containers
  * Vector
  * Array
  * RobinMap (Hash Map)
  * Iterators
  * String

* Low level systems
  * File System
  * Threads / Fibers / Synchronization Objects
  * Job System (based on Naughty Dog's)
  * Rendering (Vulkan-only for now, still reworking on my wrapper from my other project)
  
* Math Library (updated version from my old math engine)

* A Few utilities, such as Custom Delegates, Hash (FNV-1a [32, 64]), Timers etc
```

There's still a lot of work to do, and improvements to be made, some of these things being:

```
* Memory System - Have to port everything
* Rendering System - Create higher-level objects to represent the underlying API (Render Device / Context / Swapchain)
* Demos and more demos - need to put down some work showing what these systems can do
* Unit Tests
```

I'll be updating this project as I get the time to work on it.

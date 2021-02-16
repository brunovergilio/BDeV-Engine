# BDeV-Engine

This is my own Engine, which I've remade about a thousand times at this point. Right now it only contains static libraries, but I intend to start working on demos for it, as well as unit tests as it develops further.

Here's what I've got so far:

```

 - Containers
	 - Containers
	 - Vector
	 - Array
	 - RobinMap (Hash Map)
	 - Iterators
	 - String

 - Low level systems
	 - File System
	 - Threads / Fibers / Synchronization Objects
 - ~~Job System (based on Naughty Dog's)~~

 - Rendering (Vulkan-only for now, still reworking on my wrapper from my other project)
  
 - Math Library (updated version from my old math library)

 - Some other utility functions and objects
```

There's still a lot of work to do, and improvements to be made, some of these things being:

```
 - Memory System
	 - Have to rework my allocators (Linear, Stack, Pool, Heap)
	 - Make them multithreaded friendly
	 - Add more allocators maybe? (Double-ended Stack, Buddy)
 - Job System
	 - My previous fiber-based job system had a lot of flaws, will have to rework it all
 - Rendering System
	 - Add higher-level objects for ease of use and for simple demos
 - Demos
	 - Need to start working on them
 - Unit Tests
	 - When all of the above is done...
```

I'll be updating this project as I find the time to work on it.

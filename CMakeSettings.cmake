# CMakeSettings.cmake

# ===========================
# Platform-Specific Settings
# ===========================
if (WIN32)
	if(MSVC)
		# ===========================
		# Global Compiler Definitions
		# ===========================
		set(COMMON_COMPILE_DEFINITIONS
			UNICODE
			_UNICODE
			_CRT_SECURE_NO_WARNINGS
			WIN32
			_WIN32
		)

		# ===========================
		# Config-Specific Definitions
		# ===========================
		set(COMMON_DEBUG_DEFINITIONS
			DEBUG
			_DEBUG
		)

		set(COMMON_RELEASE_DEFINITIONS
			NDEBUG
		)
		
		set(COMMON_RELDBGINFO_DEFINITIONS
			NDEBUG
		)

		set(COMMON_MINSIZEREL_DEFINITIONS
			NDEBUG
		)

		# ===========================
		# Global Compiler Options
		# ===========================
		set(COMMON_COMPILE_OPTIONS
			/utf-8				# Use utf-8 encoding
			/Zc:__cplusplus		# Correct __cplusplus macro
			/std:c++20			# Use C++ 20
			/Zc:preprocessor	# Enable conforming preprocessor
			/W4					# Warning Level 4
			/WX					# Treat warnings as errors
			/Zc:wchar_t			# Treat wchar_t as a built-in type
			/Zc:inline			# Remove unref code and data
			/arch:AVX2			# Use AVX 2
			/fp:fast			# Use fast floating point
			/permissive-		# Conformance mode
			/GT					# Fiber-safe optimizations
			/GR-				# No RTTI
			/GF					# String pooling
			/MP              	# Multi-processor compilation
			/FC					# Full path in diagnostic messages
		)
		
		# ===========================
		# Config-Specific Compiler Options
		# ===========================
		set(COMMON_DEBUG_OPTIONS
			/Od					# No optimization
			/ZI					# PDB Edit and Continue
			/MDd				# MT debug lib
		)

		set(COMMON_RELEASE_OPTIONS
			/O2					# Max speed
			/Oi					# Enable intrinsic functions
			/Ot					# Fast code
			/Ob2				# Inline any suitable function
			/GL					# Whole program optimization
			/Gy					# Enable function-level linking
			/MD					# MT lib
			/Gw					# Optimize global data
		)
		
		set(COMMON_RELDBGINFO_OPTIONS
			/O2					# Max speed
			/Oi					# Enable intrinsic functions
			/Ot					# Fast code
			/Ob2				# Inline any suitable function
			/MD					# MT lib
			/Zi					# PDB
			/Gw					# Optimize global data
		)

		set(COMMON_MINSIZEREL_OPTIONS
			/O1					# Min size
			/Ob1				# Inline only explicitly marked functions
			/Os					# Favor small code
			/MD					# MT lib
			/Gw					# Optimize global data
		)
		
		# ===========================
		# Config-Specific Linker Options
		# ===========================
		set(COMMON_RELEASE_LINKER_OPTIONS
			/LTCG				# Link time optimization
		)
	endif()
endif()
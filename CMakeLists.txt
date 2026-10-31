cmake_minimum_required(VERSION 3.21)

project(DeltaWorks
	DESCRIPTION "DeltaWorks C++ framework"
	LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(DELTAWORKS_BUILD_ENGINE  "Build the DeltaWorksEngine library"  ON)
option(DELTAWORKS_BUILD_TERRAIN "Build the DeltaWorksTerrain library" ON)
option(DELTAWORKS_STATIC_RUNTIME
	"MSVC: link the static C runtime (/MT, /MTd), matching the Projector builds" ON)
set(DELTAWORKS_CXX_STANDARD 20 CACHE STRING
	"C++ standard used for DeltaWorks (Projector uses /std:c++latest)")
option(DELTAWORKS_PIC "Build position independent code" OFF)

if(DELTAWORKS_BUILD_TERRAIN AND NOT DELTAWORKS_BUILD_ENGINE)
	message(FATAL_ERROR "DELTAWORKS_BUILD_TERRAIN requires DELTAWORKS_BUILD_ENGINE")
endif()

# Projector's third configuration, OptimizedDebug (optimized, with symbols,
# defines _OPTIMIZED_DEBUG). Only added for multi-config generators when this
# is the top-level project, so consumers keep their own configuration list.
if(PROJECT_IS_TOP_LEVEL)
	get_property(_dw_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(_dw_multi_config AND NOT "OptimizedDebug" IN_LIST CMAKE_CONFIGURATION_TYPES)
		list(APPEND CMAKE_CONFIGURATION_TYPES OptimizedDebug)
		set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "" FORCE)
	endif()
	foreach(lang C CXX)
		set(CMAKE_${lang}_FLAGS_OPTIMIZEDDEBUG "${CMAKE_${lang}_FLAGS_RELWITHDEBINFO}"
			CACHE STRING "")
	endforeach()
	foreach(kind EXE SHARED STATIC MODULE)
		set(CMAKE_${kind}_LINKER_FLAGS_OPTIMIZEDDEBUG
			"${CMAKE_${kind}_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
	endforeach()
	add_compile_definitions($<$<CONFIG:OptimizedDebug>:_OPTIMIZED_DEBUG>)
	set(CMAKE_MAP_IMPORTED_CONFIG_OPTIMIZEDDEBUG RelWithDebInfo Release "")

	if(NOT CMAKE_BUILD_TYPE AND NOT _dw_multi_config)
		set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	endif()
endif()

if(DELTAWORKS_STATIC_RUNTIME AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DeltaWorksSources)

# ---------------------------------------------------------------------------
# Libraries (same layout as the .project files)
# ---------------------------------------------------------------------------
add_subdirectory(Core)
if(DELTAWORKS_BUILD_ENGINE)
	add_subdirectory(Engine)
endif()
if(DELTAWORKS_BUILD_TERRAIN)
	add_subdirectory(Terrain)
endif()

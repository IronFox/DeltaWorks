# Mirrors DeltaWorksCore.project. Keep the exclude lists in sync with it.

add_subdirectory(third_party)

deltaworks_collect_sources(core_sources "${CMAKE_CURRENT_SOURCE_DIR}"
	FIND mvc
	DIR  maker Nx third_party
	FILE remote_shell.cpp remote_shell.h)

add_library(DeltaWorksCore STATIC ${core_sources})
add_library(DeltaWorks::Core ALIAS DeltaWorksCore)
deltaworks_apply_common_settings(DeltaWorksCore)

# <source path="." include="true">: consumers include <container/...>, <math/...> etc.
target_include_directories(DeltaWorksCore PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(MSVC)
	target_compile_definitions(DeltaWorksCore PUBLIC
		_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

# Bundled libraries are OBJECT libraries, so their objects end up inside
# DeltaWorksCore.lib like in the Projector build. Package libraries are
# linked PRIVATE: CMake still propagates them to the final executable.
target_link_libraries(DeltaWorksCore PRIVATE ${DELTAWORKS_THIRD_PARTY_LIBS})
target_include_directories(DeltaWorksCore PRIVATE ${DELTAWORKS_THIRD_PARTY_INCLUDES})

find_package(Threads REQUIRED)
target_link_libraries(DeltaWorksCore PUBLIC Threads::Threads)
if(WIN32)
	target_link_libraries(DeltaWorksCore PUBLIC ws2_32)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${core_sources})
set_target_properties(DeltaWorksCore PROPERTIES FOLDER "DeltaWorks")

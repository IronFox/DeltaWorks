# Mirrors DeltaWorksCore.project. Keep the exclude lists in sync with it.

add_subdirectory(third_party)

deltaworks_collect_sources(core_sources "${CMAKE_CURRENT_SOURCE_DIR}"
	FIND mvc
	DIR  maker Nx third_party
	FILE remote_shell.cpp remote_shell.h)

# The C++ wrapper around bzip2 lives next to the library sources.
list(APPEND core_sources
	"${CMAKE_CURRENT_SOURCE_DIR}/third_party/bz2/bz2.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/third_party/bz2/bz2.h")

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

# Objects of the bundled libraries end up inside DeltaWorksCore.lib/.a,
# exactly like the Projector build.
target_link_libraries(DeltaWorksCore PRIVATE ${DELTAWORKS_THIRD_PARTY_TARGETS})

find_package(Threads REQUIRED)
target_link_libraries(DeltaWorksCore PUBLIC Threads::Threads)
if(WIN32)
	target_link_libraries(DeltaWorksCore PUBLIC ws2_32)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${core_sources})
set_target_properties(DeltaWorksCore PROPERTIES FOLDER "DeltaWorks")

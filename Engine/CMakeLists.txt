# Mirrors DeltaWorksEngine.project. Keep the exclude lists in sync with it.

deltaworks_collect_sources(engine_sources "${CMAKE_CURRENT_SOURCE_DIR}"
	FIND mvc displayConnection
	DIR  maker Nx cuda)

add_library(DeltaWorksEngine STATIC ${engine_sources})
add_library(DeltaWorks::Engine ALIAS DeltaWorksEngine)
deltaworks_apply_common_settings(DeltaWorksEngine)

target_include_directories(DeltaWorksEngine PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(DeltaWorksEngine PUBLIC DeltaWorks::Core)
if(MSVC)
	target_compile_definitions(DeltaWorksEngine PRIVATE
		_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${engine_sources})
set_target_properties(DeltaWorksEngine PROPERTIES FOLDER "DeltaWorks")

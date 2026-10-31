# Mirrors DeltaWorksTerrain.project. Keep the exclude lists in sync with it.

deltaworks_collect_sources(terrain_sources "${CMAKE_CURRENT_SOURCE_DIR}"
	FIND mvc
	DIR  maker)

add_library(DeltaWorksTerrain STATIC ${terrain_sources})
add_library(DeltaWorks::Terrain ALIAS DeltaWorksTerrain)
deltaworks_apply_common_settings(DeltaWorksTerrain)

target_include_directories(DeltaWorksTerrain PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(DeltaWorksTerrain PUBLIC DeltaWorks::Engine DeltaWorks::Core)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${terrain_sources})
set_target_properties(DeltaWorksTerrain PROPERTIES FOLDER "DeltaWorks")

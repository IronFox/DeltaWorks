# Helpers that mirror how Projector collects sources from a .project file,
# so the CMake targets compile the same files as the generated .vcxproj.
#
#   <source path="." include="true">
#       <exclude find="x"/>   -> FIND x   (relative path contains x)
#       <exclude dir="x"/>    -> DIR x    (any folder named exactly x)
#       <exclude file="x"/>   -> FILE x   (file named exactly x)
#   </source>
#
# Folders whose name starts with '.' are always skipped, like Projector does.
# Keep these lists in sync with the matching .project file.

set(DELTAWORKS_SOURCE_EXTENSIONS
	c cpp cc c++
	h hpp hh h++)

# deltaworks_collect_sources(<out-var> <root>
#     [FIND <substr>...] [DIR <name>...] [FILE <name>...])
function(deltaworks_collect_sources out_var root)
	cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "FIND;DIR;FILE")

	set(patterns)
	foreach(ext IN LISTS DELTAWORKS_SOURCE_EXTENSIONS)
		list(APPEND patterns "${root}/*.${ext}")
	endforeach()
	file(GLOB_RECURSE candidates CONFIGURE_DEPENDS
		RELATIVE "${root}" ${patterns})

	set(result)
	foreach(rel IN LISTS candidates)
		set(skip FALSE)
		get_filename_component(name "${rel}" NAME)
		get_filename_component(dir "${rel}" DIRECTORY)
		string(REPLACE "/" ";" dir_parts "${dir}")

		foreach(part IN LISTS dir_parts)
			if(part MATCHES "^\\.")
				set(skip TRUE)
			endif()
			if(part IN_LIST arg_DIR)
				set(skip TRUE)
			endif()
		endforeach()
		if(name IN_LIST arg_FILE)
			set(skip TRUE)
		endif()
		foreach(substr IN LISTS arg_FIND)
			string(FIND "${rel}" "${substr}" pos)
			if(NOT pos EQUAL -1)
				set(skip TRUE)
			endif()
		endforeach()

		if(NOT skip)
			list(APPEND result "${root}/${rel}")
		endif()
	endforeach()

	set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# Settings every Projector-generated project gets.
function(deltaworks_apply_common_settings target)
	target_compile_features(${target} PUBLIC cxx_std_${DELTAWORKS_CXX_STANDARD})
	set_target_properties(${target} PROPERTIES
		CXX_EXTENSIONS OFF
		POSITION_INDEPENDENT_CODE ${DELTAWORKS_PIC})

	if(WIN32)
		target_compile_definitions(${target} PUBLIC WIN32 UNICODE _UNICODE)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			target_compile_definitions(${target} PUBLIC PLATFORM_X64)
		else()
			target_compile_definitions(${target} PUBLIC PLATFORM_X86)
		endif()
	endif()

	if(MSVC)
		# Same as the Projector output: /W3 /sdl /MP and the noexcept warning off.
		target_compile_options(${target} PRIVATE /W3 /sdl /MP /wd26439)
		target_compile_options(${target} PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
	endif()
endfunction()

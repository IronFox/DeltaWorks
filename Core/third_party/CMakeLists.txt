# Bundled third-party C libraries, compiled into DeltaWorksCore.
# Each one is its own OBJECT library so it can later be swapped for a
# system / vcpkg / FetchContent package without touching the Core target.
# See THIRD_PARTY.md at the repository root for versions and licenses.

function(deltaworks_third_party name)
	file(GLOB sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/${name}/*.c")
	set(target deltaworks_tp_${name})
	string(REPLACE "-" "_" target "${target}")
	add_library(${target} OBJECT ${sources})
	set_target_properties(${target} PROPERTIES
		POSITION_INDEPENDENT_CODE ${DELTAWORKS_PIC}
		FOLDER "DeltaWorks/third_party")
	if(MSVC)
		target_compile_definitions(${target} PRIVATE
			_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
		target_compile_options(${target} PRIVATE /W3)
	else()
		# Old upstream code: keep the build log readable.
		target_compile_options(${target} PRIVATE -w)
	endif()
	set(DELTAWORKS_THIRD_PARTY_TARGETS
		${DELTAWORKS_THIRD_PARTY_TARGETS} ${target} PARENT_SCOPE)
endfunction()

set(DELTAWORKS_THIRD_PARTY_TARGETS)
deltaworks_third_party(zlib)
deltaworks_third_party(png)
deltaworks_third_party(jpeg)
deltaworks_third_party(bz2)
deltaworks_third_party(open-simplex-noise)

# libpng includes "../zlib/zlib.h" directly; nothing else to wire up.
if(UNIX)
	target_link_libraries(deltaworks_tp_png PUBLIC m)
	target_link_libraries(deltaworks_tp_open_simplex_noise PUBLIC m)
endif()

set(DELTAWORKS_THIRD_PARTY_TARGETS ${DELTAWORKS_THIRD_PARTY_TARGETS} PARENT_SCOPE)

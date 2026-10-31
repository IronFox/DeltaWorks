# Third-party libraries used by DeltaWorksCore.
#
# zlib, libpng, libjpeg and bzip2 come from packages (vcpkg, or any other
# provider of the standard find_package() modules) or from the copies bundled
# in this folder, controlled by DELTAWORKS_THIRD_PARTY:
#   AUTO     - use packages if all four are found, otherwise the bundled copies
#   PACKAGES - require packages
#   BUNDLED  - always use the bundled copies
#
# Bundled versions (unmodified upstream sources, see VERSIONS.md):
# zlib 1.3.2, libpng 1.6.59, IJG libjpeg 10, bzip2 1.0.8.
# open-simplex-noise (public domain) is always bundled.
#
# Sets DELTAWORKS_THIRD_PARTY_LIBS (link) and DELTAWORKS_THIRD_PARTY_INCLUDES
# (private include dirs) in the parent scope.

set(DELTAWORKS_THIRD_PARTY AUTO CACHE STRING
	"Where zlib/libpng/libjpeg/bzip2 come from: AUTO, PACKAGES or BUNDLED")
set_property(CACHE DELTAWORKS_THIRD_PARTY PROPERTY STRINGS AUTO PACKAGES BUNDLED)

function(deltaworks_bundled name)
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
	set(DELTAWORKS_THIRD_PARTY_LIBS ${DELTAWORKS_THIRD_PARTY_LIBS} ${target} PARENT_SCOPE)
endfunction()

set(DELTAWORKS_THIRD_PARTY_LIBS)
set(DELTAWORKS_THIRD_PARTY_INCLUDES)

set(_dw_use_packages FALSE)
if(NOT DELTAWORKS_THIRD_PARTY STREQUAL "BUNDLED")
	if(DELTAWORKS_THIRD_PARTY STREQUAL "PACKAGES")
		set(_dw_required REQUIRED)
	else()
		set(_dw_required QUIET)
	endif()
	find_package(ZLIB ${_dw_required})
	find_package(PNG ${_dw_required})
	find_package(JPEG ${_dw_required})
	find_package(BZip2 ${_dw_required})
	if(ZLIB_FOUND AND PNG_FOUND AND JPEG_FOUND AND BZIP2_FOUND)
		set(_dw_use_packages TRUE)
	endif()
endif()

if(_dw_use_packages)
	message(STATUS "DeltaWorks: using zlib ${ZLIB_VERSION_STRING}, libpng ${PNG_VERSION_STRING}, "
		"libjpeg ${JPEG_VERSION}, bzip2 ${BZIP2_VERSION_STRING} from packages")
	list(APPEND DELTAWORKS_THIRD_PARTY_LIBS PNG::PNG JPEG::JPEG BZip2::BZip2 ZLIB::ZLIB)
else()
	if(DELTAWORKS_THIRD_PARTY STREQUAL "AUTO")
		message(STATUS "DeltaWorks: zlib/libpng/libjpeg/bzip2 packages not all found, using the bundled copies")
	else()
		message(STATUS "DeltaWorks: using the bundled zlib/libpng/libjpeg/bzip2")
	endif()
	deltaworks_bundled(zlib)
	deltaworks_bundled(png)
	deltaworks_bundled(jpeg)
	deltaworks_bundled(bz2)
	# The DeltaWorks sources include <png.h>, <jpeglib.h> and <bzlib.h>.
	list(APPEND DELTAWORKS_THIRD_PARTY_INCLUDES
		"${CMAKE_CURRENT_SOURCE_DIR}/png"
		"${CMAKE_CURRENT_SOURCE_DIR}/jpeg"
		"${CMAKE_CURRENT_SOURCE_DIR}/bz2"
		"${CMAKE_CURRENT_SOURCE_DIR}/zlib")
	target_include_directories(deltaworks_tp_png PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/zlib")
	target_compile_definitions(deltaworks_tp_png PRIVATE PNG_ARM_NEON_OPT=0)
	if(UNIX)
		target_compile_definitions(deltaworks_tp_zlib PRIVATE HAVE_UNISTD_H)
		target_link_libraries(deltaworks_tp_png PUBLIC m)
	endif()
endif()

deltaworks_bundled(open-simplex-noise)
if(UNIX)
	target_link_libraries(deltaworks_tp_open_simplex_noise PUBLIC m)
endif()

set(DELTAWORKS_USING_PACKAGES ${_dw_use_packages} PARENT_SCOPE)
set(DELTAWORKS_THIRD_PARTY_LIBS ${DELTAWORKS_THIRD_PARTY_LIBS} PARENT_SCOPE)
set(DELTAWORKS_THIRD_PARTY_INCLUDES ${DELTAWORKS_THIRD_PARTY_INCLUDES} PARENT_SCOPE)

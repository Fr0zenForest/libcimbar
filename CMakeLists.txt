cmake_minimum_required(VERSION 3.10)

project ( libcimbar )
enable_testing()

option(BUILD_DESKTOP_TOOLS "Build OpenGL/GLFW desktop tools (cimbar_send/cimbar_recv)" ON)
option(WIN7_COMPAT "Target Windows 7 API (_WIN32_WINNT=0x0601)" OFF)
option(STATIC_MSVC_RUNTIME "Use static MSVC runtime (/MT, /MTd)" OFF)

if (MSVC)
    add_compile_options(/FI "iso646.h")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${libcimbar_SOURCE_DIR}/dist" CACHE PATH "..." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
add_definitions("-DLIBCIMBAR_PROJECT_ROOT=\"${libcimbar_SOURCE_DIR}\"")

if(NOT CMAKE_CONFIGURATION_TYPES AND "${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

if(NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99 -Os -fPIC")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Wall -Wextra -Wpedantic -Wdeprecated -g -O2 -fPIC")
endif()

if(WIN32 AND WIN7_COMPAT)
	add_compile_definitions(WINVER=0x0601 _WIN32_WINNT=0x0601)
endif()

if(MSVC AND STATIC_MSVC_RUNTIME)
	foreach(flag_var
		CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL
	)
		string(REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	endforeach()
endif()

if(DEFINED USE_WASM)  # wasm build needs OPENCV_DIR defined
	set(DISABLE_TESTS true)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGF256_TARGET_MOBILE")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os")

	file( GLOB OPENCV_LIBS "${OPENCV_DIR}/opencv-build-wasm/build_wasm/lib/*.a" )
	file( GLOB opencv_include_modules "${OPENCV_DIR}/modules/*/include" )
	include_directories(
		${OPENCV_DIR}/include/
		${OPENCV_DIR}/opencv-build-wasm/build_wasm/
		${opencv_include_modules}
	)
else()  # if not wasm, go find opencv. 3 or 4 should both work
	find_package(OpenCV REQUIRED)
	include_directories(${OpenCV_INCLUDE_DIRS})
endif()

if(DEFINED BUILD_PORTABLE_LINUX)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPENCV4 REQUIRED opencv4)
	link_directories(${OPENCV4_STATIC_LIBRARY_DIRS})

	# statically link c++/gcc (not libc!!)
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -static-libstdc++ -static-libgcc")
endif()

if(NOT DEFINED OPENCV_LIBS)
	if(DEFINED OpenCV_LIBS AND NOT "${OpenCV_LIBS}" STREQUAL "")
		set(OPENCV_LIBS ${OpenCV_LIBS})
	else()
		set(OPENCV_LIBS "opencv_calib3d" "opencv_imgcodecs" "opencv_imgproc" "opencv_photo" "opencv_core" ${OPENCV4_STATIC_LIBRARIES})
	endif()
endif()

if(NOT DEFINED CPPFILESYSTEM)
	set(CPPFILESYSTEM "stdc++fs")
endif()

if(MSVC)
	set(CPPFILESYSTEM "")
endif()

if(WIN32)
	set(OPENGL_LIB "opengl32")
	if(BUILD_DESKTOP_TOOLS)
		find_package(glfw3 CONFIG REQUIRED)
		if(TARGET glfw)
			set(GLFW_LIB glfw)
		elseif(TARGET glfw3)
			set(GLFW_LIB glfw3)
		elseif(TARGET glfw3::glfw)
			set(GLFW_LIB glfw3::glfw)
		else()
			set(GLFW_LIB "glfw3")
		endif()
	else()
		set(GLFW_LIB "")
	endif()
	if(BUILD_DESKTOP_TOOLS)
		find_package(GLEW REQUIRED)
		set(GLEW_LIB GLEW::GLEW)
	else()
		set(GLEW_LIB "")
	endif()
else()
	set(OPENGL_LIB "GL")
	set(GLFW_LIB "glfw")
	set(GLEW_LIB "")
endif()

set (PROJECTS
	src/lib/bit_file
	src/lib/chromatic_adaptation
	src/lib/cimb_translator
	src/lib/compression
	src/lib/encoder
	src/lib/extractor
	src/lib/fountain
	src/lib/image_hash
	src/lib/serialize
	src/lib/util

	src/third_party_lib/base91
	src/third_party_lib/cxxopts
	src/third_party_lib/intx
	src/third_party_lib/libcorrect
	src/third_party_lib/libpopcnt
	src/third_party_lib/wirehair
	src/third_party_lib/zstd
)

if(DEFINED USE_WASM OR BUILD_DESKTOP_TOOLS)
set(PROJECTS
	${PROJECTS}

	src/lib/cimbar_js
	src/lib/gui
)
endif()

if(NOT DEFINED USE_WASM)
set(PROJECTS
	${PROJECTS}

	src/exe/cimbar
	src/exe/cimbar_extract
	src/exe/build_image_assets
)

if(BUILD_DESKTOP_TOOLS)
set(PROJECTS
	${PROJECTS}

	src/exe/cimbar_recv
	src/exe/cimbar_recv2
	src/exe/cimbar_send
)
endif()
endif()

include_directories(
	${libcimbar_SOURCE_DIR}/src/lib
	${libcimbar_SOURCE_DIR}/src/third_party_lib
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()

cmake_minimum_required(VERSION 3.25)

project(rewm VERSION 0.0)

#set(CMAKE_MODULE_PATH /usr/local/lib/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -Wall -Wpedantic")

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_INCLUDE_PATH /usr/include)



find_path(X11_Xrender_INCLUDE_PATH X11/extensions/Xrender.h ${X11_INC_SEARCH_PATH})
find_path(X11_Xpm_INCLUDE_PATH X11/xpm.h ${X11_INC_SEARCH_PATH})

find_library(IMLIB2_LIBRARY NAMES Imlib2)
if(NOT IMLIB2_LIBRARY)
    message(FATAL_ERROR "Imlib2 library not found")
endif()

find_package(X11 REQUIRED )
find_package(JPEG REQUIRED )

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")
message(STATUS "X11_Xrender_FOUND = ${X11_Xrender_FOUND}")
message(STATUS "X11_Xpm_FOUND = ${X11_Xpm_FOUND}")


include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})

include_directories(${JPEG_INCLUDE_DIR})
link_directories(${JPEG_LIBRARIES})

include_directories(${X11_Xrender_INCLUDE_DIR})
link_directories(${X11_Xrender_LIB})

include_directories(/usr/include/freetype2)
include_directories(${X11_Xft_INCLUDE_DIR})
link_directories(${X11_Xft_LIB})

include_directories(${X11_Xpm_INCLUDE_DIR})
link_directories(${X11_Xpm_LIB})

include_directories(${X11_Xinerama_INCLUDE_DIR})
link_directories(${X11_Xinerama_LIB})

add_subdirectory(src)

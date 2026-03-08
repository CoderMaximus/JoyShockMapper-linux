cmake_minimum_required (VERSION 3.28)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(SDL OR NOT DEFINED SDL)
	set(PROJECT_NAME "JoyShockMapper_SDL2")
else()
	set(PROJECT_NAME "JoyShockMapper_JSL")
endif()

project (${PROJECT_NAME} CXX)

# Turning off extensions avoids and issue with the clang 16 compiler with modules
set(CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include (cmake/LinuxConfig.cmake)
include (cmake/WindowsConfig.cmake)
include (cmake/CPM.cmake)
include (cmake/GetGitRevisionDescription.cmake)

add_subdirectory (JoyShockMapper)

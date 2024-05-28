cmake_minimum_required(VERSION 3.18)
project(PhysXParser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# For clangd (linting)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to Release.")
	message(STATUS "To Debug, run `cmake -DCMAKE_BUILD_TYPE=Debug`")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2 /sdl /Oi /W3 /std:c++17")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -ggdb")
endif()


file(GLOB PHYSX_PARSER_SOURCES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/heightfield/*.cpp
)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/include" 
	"${CMAKE_CURRENT_SOURCE_DIR}/lib/Binary-Reader/include"
)

add_library(PhysxParser STATIC ${PHYSX_PARSER_SOURCES})

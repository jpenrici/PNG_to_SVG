cmake_minimum_required(VERSION 3.20)

project(SimplePng2Svg LANGUAGES CXX)

set(PROGRAM_NAME png2svg)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
file(GLOB SOURCES src/*.cpp)

set(RESOURCE_FILES images/test.png)

add_executable(${PROGRAM_NAME} ${SOURCES})

find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
  message(STATUS "ZLIB found:")
  message(STATUS "  Include Directories: ${ZLIB_INCLUDE_DIRS}")
  message(STATUS "  Libraries: ${ZLIB_LIBRARIES}")
else()
  message(FATAL_ERROR "ZLIB not found. Please ensure the ZLIB library is installed on your system.")
endif()

target_link_libraries (${PROGRAM_NAME} PUBLIC ZLIB::ZLIB)

file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Resources)

install(TARGETS ${PROGRAM_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

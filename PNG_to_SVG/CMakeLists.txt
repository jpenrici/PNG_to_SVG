cmake_minimum_required(VERSION 3.5)

project(SimplePng2Svg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    svg.h
    png2svg.h png2svg.cpp
)

set(RESOURCE_FILES
    images/test.png
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
)

find_package(ZLIB)

target_link_libraries (
    ${PROJECT_NAME} PUBLIC
    ZLIB::ZLIB
)

file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Resources)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

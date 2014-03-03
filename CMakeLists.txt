cmake_minimum_required(VERSION 2.8)
Project(lzw)

set(SOURCES
    src/main.c
    src/LZW.c
    src/Dictionary.c
)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)

add_executable (
    lzw
    WIN32
    MACOSX_BUNDLE
    ${SOURCES}
)
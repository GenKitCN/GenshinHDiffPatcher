cmake_minimum_required(VERSION 3.23)
project(genshin_patcher)

set(CMAKE_CXX_STANDARD 17)
set(ZIPPER_PATH includes/zipper)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
endif()

add_subdirectory(includes/11Zip)

include_directories(
        includes
        includes/HDiffPatch
        includes/json/include
        includes/11Zip/include
        includes/spdlog/include
        includes/cxxopts/include
        includes/HDiffPatch/libHDiffPatch
)

add_library(HPatchFile STATIC includes/HDiffPatch/file_for_patch.h includes/HDiffPatch/file_for_patch.c)
add_library(HPatchPatchLib STATIC
        includes/HDiffPatch/libHDiffPatch/HPatch/patch.h
        includes/HDiffPatch/libHDiffPatch/HPatch/patch.c
        )

add_executable(genshin_patcher main.cpp)
target_link_libraries(genshin_patcher PRIVATE HPatchFile HPatchPatchLib elzip)

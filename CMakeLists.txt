cmake_minimum_required(VERSION 3.23)
project(genshin_patcher)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(includes/11Zip)

include_directories(
        includes/json/include
        includes/cxxopts/include
        includes/HDiffPatch/libHDiffPatch
        includes/HDiffPatch/dirDiffPatch
)

add_executable(genshin_patcher main.cpp)
target_link_libraries(genshin_patcher elzip)

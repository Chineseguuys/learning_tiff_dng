cmake_minimum_required(VERSION 3.10)

project(unprocess_raw)

#find_package 无法直接找到使用 .pc 作为 config 的软件包，需要使用 PkgConfig 模块来寻找
find_package(PkgConfig REQUIRED)
find_package(spdlog REQUIRED)
pkg_check_modules(libraw_r.pc REQUIRED libraw_r)

# 设置你的项目的源文件
set(SOURCE_FILES
    parse_tiff.cpp
    ./src/parse_ifd.cpp
    ./src/tiff_ifd.cpp
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME} 
    raw_r
    spdlog::spdlog
)

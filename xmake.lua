-- 使用 require 导入 xmake 库
add_requires("spdlog", "libraw_r")

-- 定义目标项目
target("unprocess_raw")
    set_kind("binary") -- 设置目标类型为二进制可执行文件
    add_files("tiff_read.cpp") -- 添加源文件

    -- 链接库
    add_packages("spdlog", "libraw_r")

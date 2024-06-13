# 指定编译器 警告如果子文件包含project(xxxx)语句会陷入死循环
set(CMAKE_C_COMPILER /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_STRIP /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-strip)
set(CMAKE_FIND_ROOT_PATH /opt/imx6ul-gcc-7.5.0/bin/)


# 正则表达式从当前路径解析出当前目录名
string( REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} )
# 当前目录名作为输出目标文件名
set(output_name ${CURRENT_FOLDER})

aux_source_directory(. source_files)
add_executable(${output_name} ${source_files})


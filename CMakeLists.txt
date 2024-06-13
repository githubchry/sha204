
cmake_minimum_required(VERSION 3.10)
project(sha204 VERSION 1.0.0)


include(utils.cmake)


# 声明c++17语法支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD  11)


# 打开所有编译告警
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# 关闭changed in GCC 7.1的编译警告
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# make编译时可查看包含的头文件路径，库文件等信息
set(CMAKE_VERBOSE_MAKEFILE on)


########################
#[CMake中使用pthread的坑与解决方案](https://zhuanlan.zhihu.com/p/128519905)
#GCC潜在的bug: target_link_libraries(xxxx PRIVATE pthread) => pthread会扩展为-lpthread，既仅对链接阶段生效
#比如rysingleton.h里面使用了std::call_once实现单例, std::call_once里面没有显式调用pthread, 所以cmake不会主动引用pthread, 导致运行时抛出std::system_error异常
#CMAKE中提供了单独的Threads库来解决这个问题
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")

# 测试发现编出来的程序大小: 191856 => 191880 反正也不占用啥  干脆link_libraries全局引用 一劳永逸
########################


########################
#-fPIC 作用于编译阶段，告诉编译器产生与位置无关代码(Position-Independent Code)，则产生的代码中，没有绝对地址，全部使用相对地址，
#故而代码可以被加载器加载到内存的任意位置，都可以正确的执行。这正是共享库所要求的，共享库被加载时，在内存的位置不是固定的。
#[gcc编译参数-fPIC的一些问题](http://blog.sina.com.cn/s/blog_54f82cc201011op1.html)
#要求仅对共享库指定，如果对可执行文件指定可能会影响性能
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
########################

########################
#有时我们的程序会定义一些暂时使用不上的功能和函数，虽然我们不使用这些功能和函数，但它们往往会浪费我们的ROM和RAM的空间。
#这在使用静态库时，体现的更为严重。有时，我们只使用了静态库仅有的几个功能，但是系统默认会自动把整个静态库全部链接到可执行程序中，造成可执行程序的大小大大增加。
#为了解决前面分析的问题，我们引入了以下的几个参数。
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS}  -Wl,-gc-sections")
#GCC链接操作是以section作为最小的处理单元，只要一个section中的某个符号被引用，该section就会被加入到可执行程序中去。
#因此，GCC在编译时可以使用 -ffunction-sections和 -fdata-sections 将每个函数或符号创建为一个sections，其中每个sections名与function或data名保持一致。
#而在链接阶段， -Wl,–gc-sections 指示链接器去掉不用的section（其中-Wl, 表示后面的参数 -gc-sections 传递给链接器），这样就能减少最终的可执行程序的大小了。
########################


########################
# 链接选项 -rdynamic 针对可执行文件，用来通知链接器将所有符号导出到动态符号表中(默认情况下符号只会从共享库中导出)
# 在gcc中 -rdynamic 与 -Wl,-E 和 -Wl,--export-dynamic 的作用等价。
# 使用场景：bin需要打印backtrace时(可看到更详细的符号信息);.so反向调用bin时(extern); => 总之就是针对bin
# -ldl 链接选项针对dlopen, dlerror, dlsym, dlclose等动态库使用接口，使用-rdynamic的场景经常都会用到-ldl
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
########################

########################################################################################################################

if (0)
    add_definitions(-DASAN)
    ########################
    # 内存泄漏检查工具 asan
    # -g -fsanitize=address开启内存泄漏检测功能，用-fno-omit-frame-pointer编译，以得到更容易理解stack trace。
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -fno-omit-frame-pointer -ldl -lpthread -lrt")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -fsanitize=address -fno-omit-frame-pointer -ldl -lpthread -lrt")
    # 有些工具链的asan库可能会依赖： -ldl -lpthread -lrt
    #  ！！！一定要去掉-O2参数！！！
    ########################
else ()
    # 优化参数-O2 该优化选项会牺牲部分编译速度，除了执行-O1所执行的所有优化之外，还会采用几乎所有的目标配置支持的优化算法，用以提高目标代码的运行速度。
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()


########################
# 覆盖率检查工具 gcov      cmake .. -DCMAKE_BUILD_TYPE=Debug
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage -lgcov")
########################


########################
# 性能测试工具 gprof     cmake .. -DCMAKE_BUILD_TYPE=Debug
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pg")
# 无法测试动态加载库(dlopen)内部的代码 所以要求在静态链接模式下进行
########################

########################################################################################################################
# 如果未指定CMAKE_BUILD_TYPE，默认设置为Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("设置默认CMAKE_BUILD_TYPE为Debug")
endif ()
########################################################################################################################

# 如果是MAC系统，设置RPATH还需要set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "./lib")

if (NOT WSL)
    # 指定编译器 警告如果子文件包含project(xxxx)语句会陷入死循环
    set(CMAKE_C_COMPILER /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-g++)
    SET(CMAKE_STRIP /opt/imx6ul-gcc-7.5.0/bin/arm-linux-gnueabihf-strip)
    set(CMAKE_FIND_ROOT_PATH /opt/imx6ul-gcc-7.5.0/bin/)
    # 依赖库路径
    include_directories(new-root/imx/include)
    link_directories(new-root/imx/lib)

    set(DYNAMIC_LIBRARY_PATH  ${CMAKE_SOURCE_DIR}/dist/imx-lib)
else ()
    add_definitions(-DWSL)
    include_directories(new-root/wsl/include)
    link_directories(new-root/wsl/lib)

    set(DYNAMIC_LIBRARY_PATH  ${CMAKE_SOURCE_DIR}/dist/wsl-lib)
endif ()


message(${CMAKE_BUILD_TYPE})
message(${CMAKE_C_COMPILER})
message(${CMAKE_CXX_COMPILER})




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


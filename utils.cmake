
# 遍历获取_dir下所有target(bin/lib), 输出到_result
function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach (_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach ()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)

    # PARENT_SCOPE 在父作用域(parent scope)中设置变量
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()


# 重新定义当前目标的源文件的__FILE__宏   可配合get_all_targets使用
function(redefine_file_macro target)
    #获取当前目标的所有源文件
    get_target_property(source_files "${target}" SOURCES)
    #遍历源文件
    foreach (sourcefile ${source_files})
        #获取当前源文件的编译参数
        get_property(defs SOURCE "${sourcefile}" PROPERTY COMPILE_DEFINITIONS)
        #获取当前文件的绝对路径
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)

        # ${filepath} => /mnt/d/codes/git/maintain/maintain/app/maintain/webserver/ryjwt.cpp
        # [\S]+\/([\w\.-]+\/[\w\.-]+)$
        # ==转义==> [\\S]+\\/([\\w\\.-]+\\/[\\w\\.-]+)$
        # cmake的正则不支持简写字符集 \S \w  => [a-zA-Z0-9\.-\\_]++\/([a-zA-Z0-9\.\-_]+\/[a-zA-Z0-9\.\-_]+)$
        # string(REGEX MATCH "[a-zA-Z0-9\\.\\_-]+\\/([a-zA-Z0-9\\.\\_-]+\\/[a-zA-Z0-9\\.\\_-]+)$" outstr ${filepath})
        # message("${outstr}")    # 即${CMAKE_MATCH_0} => maintain/webserver/ryjwt.cpp
        # message("${CMAKE_MATCH_1}") # webserver/ryjwt.cpp

        if (${CMAKE_C_COMPILER} STREQUAL "/usr/bin/cc")

            # 这种方式更利于IDE调试一键跳转, 非交叉编译调试的时候使用

            #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
            string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relative_path ${filepath})

            #将我们要加的编译参数(__FILE__定义)添加到原来的编译参数里面
            list(APPEND defs "__FILE__=\"${relative_path}\"")

        elseif ("${filepath}" MATCHES "[a-zA-Z0-9\\.\\_-]+\\/([a-zA-Z0-9\\.\\_-]+\\/[a-zA-Z0-9\\.\\_-]+)$")
            # 这种方式更省空间

            # message("${CMAKE_MATCH_0}")
            # message("${CMAKE_MATCH_1}")

            #将我们要加的编译参数(__FILE__定义)添加到原来的编译参数里面
            list(APPEND defs "__FILE__=\"${CMAKE_MATCH_1}\"")
        else ()
            #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
            string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relative_path ${filepath})

            #将我们要加的编译参数(__FILE__定义)添加到原来的编译参数里面
            list(APPEND defs "__FILE__=\"${relative_path}\"")
        endif ()


        #重新设置源文件的编译参数
        set_property(SOURCE "${sourcefile}" PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach ()
endfunction()

# redefine_file_macro里面重定义了__FILE__ 会产生redefined警告，忽略之
add_definitions(-Wno-builtin-macro-redefined)


# 获取git短版本 => git show -s --format=%h
function(get_git_version GIT_DIR GIT_VERSION)
    find_package(Git)
    if (GIT_FOUND)
        # 执行一个子进程
        execute_process(
                COMMAND ${GIT_EXECUTABLE} show -s --format=%h # 命令
                OUTPUT_VARIABLE version        # 输出字符串存入变量
                OUTPUT_STRIP_TRAILING_WHITESPACE    # 删除字符串尾的换行符
                WORKING_DIRECTORY ${GIT_DIR} # 执行路径
        )

        # PARENT_SCOPE 在父作用域(parent scope)中设置变量
        set(${GIT_VERSION} ${version} PARENT_SCOPE)

    endif ()
endfunction()


# 获取svn版本 参考/usr/share/cmake-3.21/Modules/FindSubversion.cmake
function(get_svn_version dir SVN_VERSION)
    find_package(Subversion)
    if (Subversion_FOUND)
        cmake_parse_arguments(
                "Subversion_WC_INFO"
                "IGNORE_SVN_FAILURE"
                "" ""
                ${ARGN}
        )

        # the subversion commands should be executed with the C locale, otherwise
        # the message (which are parsed) may be translated, Alex
        set(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
        set(ENV{LC_ALL} C)

        execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} info ${dir}
                OUTPUT_VARIABLE SVN_WC_INFO
                ERROR_VARIABLE Subversion_svn_info_error
                RESULT_VARIABLE Subversion_svn_info_result
                OUTPUT_STRIP_TRAILING_WHITESPACE)

        if (${Subversion_svn_info_result} EQUAL 0)
            string(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
                    "\\2" SVN_WC_REVISION "${SVN_WC_INFO}")

            # PARENT_SCOPE 在父作用域(parent scope)中设置变量
            set(${SVN_VERSION} ${SVN_WC_REVISION} PARENT_SCOPE)
        endif ()

        # restore the previous LC_ALL
        set(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})
    endif ()
endfunction()



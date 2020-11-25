cmake_minimum_required(VERSION 3.15)
project(sms LANGUAGES CXX)

option(BUILD_WITH_DEBUG "Build Debug" ON)
if(BUILD_WITH_DEBUG)
    message("####################### BUILD WITH DEBUG #######################")
    add_compile_options(-g3 -O0)
    add_definitions(-D__SMS_DEBUG__=1)
else()
    message("####################### BUILD WITH RELEASE #######################")
    add_compile_options(-O3)    
endif()

# 设置rpath
set(CMAKE_SKIP_BUILD_RPATH TRUE)                 
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)          
set(CMAKE_INSTALL_RPATH "")                       
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)    

# 设置git版本头文件
find_package(Git)
execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%ce
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  REPO_AUTHOR_EMAIL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(WRITE ${PROJECT_SOURCE_DIR}/src/common/repo_version.h
    "#ifndef SMS_REPO_VERSION_H\n#define SMS_REPO_VERSION_H\n#define REPO_VERSION \"${GIT_REPO_VERSION}\"\n#define REPO_DATE \"${GIT_REPO_DATE}\"\n#define REPO_HASH \"${GIT_REPO_HASH}\"\n#define REPO_AUTHOR_EMAIL \"${REPO_AUTHOR_EMAIL}\"\n#endif\n"
)

add_subdirectory(3rdparty/libuv)
add_subdirectory(src)
file(
    GLOB_RECURSE SOURCE_FILES
    LIST_DIRECTORIES false
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_11)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Wall)


# 设置第三方依赖路径
set(3RD_PATH ${CMAKE_BINARY_DIR}/3rdparty)

# LIBUV
add_dependencies(${CMAKE_PROJECT_NAME} uv_a)
target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC ${3RD_PATH}/libuv)
target_link_libraries(${CMAKE_PROJECT_NAME} libuv_a.a)
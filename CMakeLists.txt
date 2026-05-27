cmake_minimum_required(VERSION 3.16)
project(TowerDefense LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用本地 SFML
set(SFML_DIR "D:/SFML-2.6.1/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS system window graphics audio REQUIRED)

# 源文件
file(GLOB_RECURSE SOURCES src/*.cpp main/*.cpp)
list(APPEND SOURCES src/PathFinder.c)

# 可执行文件
add_executable(tower_defense ${SOURCES})

target_include_directories(tower_defense PRIVATE include src)

target_link_libraries(tower_defense PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)

# Windows 上复制 SFML DLL 和 assets 到可执行文件目录
if(WIN32)
    add_custom_command(TARGET tower_defense POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/SFML-2.6.1/bin/sfml-system-2.dll" $<TARGET_FILE_DIR:tower_defense>  #输入正确的路径
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/SFML-2.6.1/bin/sfml-window-2.dll" $<TARGET_FILE_DIR:tower_defense>  #输入正确的路径
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/SFML-2.6.1/bin/sfml-graphics-2.dll" $<TARGET_FILE_DIR:tower_defense>  #输入正确的路径
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/SFML-2.6.1/bin/sfml-audio-2.dll" $<TARGET_FILE_DIR:tower_defense>  #输入正确的路径
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:tower_defense>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/fonts" "$<TARGET_FILE_DIR:tower_defense>/fonts"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/sound" "$<TARGET_FILE_DIR:tower_defense>/sound"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures" "$<TARGET_FILE_DIR:tower_defense>/textures"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures/background" "$<TARGET_FILE_DIR:tower_defense>/textures/background"
    )
endif()

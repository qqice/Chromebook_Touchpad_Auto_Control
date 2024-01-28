cmake_minimum_required(VERSION 3.7)
project(ChromeBook_Touchpad_Auto_Control)

set(CMAKE_CXX_STANDARD 11)

add_executable(ChromeBook_Touchpad_Auto_Control WIN32 main.cpp)

# 添加清单文件
set_target_properties(ChromeBook_Touchpad_Auto_Control PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
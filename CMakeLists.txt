cmake_minimum_required (VERSION 3.8)

project ("rageAm")

add_compile_definitions(IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_rage/ImGuiConfigRage.h")

add_subdirectory ("src")
add_subdirectory ("injector")
add_subdirectory ("vendor")

set_property(TARGET imgui PROPERTY CXX_STANDARD 20)

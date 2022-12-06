cmake_minimum_required (VERSION 3.8)

# Definde dynamic library
add_executable(rageInjector	"main.cpp")

set_property(TARGET rageInjector PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")

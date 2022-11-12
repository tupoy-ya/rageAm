cmake_minimum_required (VERSION 3.8)

include_directories(${CMAKE_CURRENT_LIST_DIR})

# Boost Libary
set(BOOST_ROOT $ENV{Boost_ROOT})
# set(Boost_USE_STATIC_LIBS OFF) 
find_package(Boost REQUIRED COMPONENTS stacktrace_noop REQUIRED)

# Definde dynamic library
add_library(rageAm SHARED 
	"main.cpp" "main.h" "Memory/Hooking.h" "Memory/Hooking.cpp" "Logger.h" "Logger.cpp")

# Link static libraries
target_link_libraries(rageAm PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/minhook-1.3.3/lib/libMinHook-x64-v141-md.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/scripthook/lib/ScriptHookV.lib" 
    Boost::stacktrace_noop)

# Set latest c++ standard
set_property(TARGET rageAm PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")

# Copy to GTA V directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/rageAm.dll 
        "$ENV{GTAPath}/rageAm.asi")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/rageAm.pdb 
        "$ENV{GTAPath}/rageAm.pdb")

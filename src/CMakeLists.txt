cmake_minimum_required (VERSION 3.8)

# include_directories(${CMAKE_CURRENT_LIST_DIR})

# Boost Libary
set(BOOST_ROOT $ENV{Boost_ROOT})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)

# Definde dynamic library
add_library(rageAm SHARED 
	"main.cpp" "main.h" 
    "Memory/Hooking.h" "Memory/Hooking.cpp" 
    "Logger.h" "Logger.cpp" 
    "Component.h"
    "ComponentMgr.h" "ComponentMgr.cpp"
    "ImGui/imgui_impl_gta.cpp" "ImGui/imgui_impl_gta.h" 
    "ImGui/ImGuiGta.h" "ImGui/ImGuiGta.cpp" 
    "GtaCommon.h"
    "Graphics/GtaDirectX.h" 
    "Platform/GtaWindow.h" 
    "Rage/fwFsm.h"
    "Rage/CVehicleFactory.h"
    "Rage/Streaming.h"
    "Rage/Pool.h")

# target_include_directories(rageAm PUBLIC "../vendor/imgui-1.88")

target_include_directories(rageAm PUBLIC ${Boost_INCLUDE_DIRS})

# Link static libraries
target_link_libraries(rageAm PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/minhook-1.3.3/lib/libMinHook-x64-v141-md.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/scripthook/lib/ScriptHookV.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/directxtk/lib/DirectXTK.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/backtrace/libbacktrace.a" 
    d3d11.lib
    imgui
    ${Boost_LIBRARIES})

# Set latest c++ standard
set_property(TARGET rageAm PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")

# Eject DLL and PDB (if stack trace was used) if already loaded
add_custom_command(TARGET ${PROJECT_NAME} 
    PRE_BUILD
    COMMAND 
    
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe
    -e rageAm.dll 
)
add_custom_command(TARGET ${PROJECT_NAME} 
    PRE_BUILD
    COMMAND 
    
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe
    -e rageAm.pdb 
)

# Copy to temp directory, otherwise we can't compile DLL because its used by GTA.

# DLL
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND 
    ${CMAKE_COMMAND} -E 
   
    copy
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/rageAm.dll 
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp/rageAm.dll
)

# PDB
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E 
   
    copy
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/rageAm.pdb 
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp/rageAm.pdb
)

# Inject DLL
if(NOT ${CMAKE_PRESET_NAME} MATCHES "x64-debug-unload-noinject")
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND
   
        ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe
        -i ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp/rageAm.dll 
    )
endif()

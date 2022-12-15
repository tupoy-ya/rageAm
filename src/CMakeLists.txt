cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi -O2")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

add_library(rageAm SHARED 
	"main.cpp"

    "Logger.h"

    "imgui_rage/ImGuiConfigRage.h"
    
    
    "imgui_rage/ImGuiRage.h"
    "imgui_rage/ImGuiRage.cpp"
    "imgui_rage/ImGuiApp.h" 
    "imgui_rage/ImGuiAppMgr.h" 
    "imgui_rage/ImGuiExtensions.h"

    "imgui_rage/sys_apps/ImGuiApp_MainWindow.h"
    "imgui_rage/sys_apps/ImGuiApp_Toolbar.h" 
    "imgui_rage/sys_apps/ImGuiApp_MaterialEditor.h"

    "memory/gmAddress.h"
    "memory/gmAddress.cpp"
    "memory/gmHook.cpp"
    "memory/gmHook.h"
    "memory/gmScanner.cpp"
    "memory/gmScanner.h"
    "memory/pattern.h"
    "memory/pattern.cpp"
    "memory/gmHelper.h"

    "rage/atArray.h"
    "rage/CVehicleFactory.h"
    "rage/datBase.h"
    "rage/fiDevice.h"
    "rage/fiPackfile.h"
    "rage/fiPackfile.cpp"
    "rage/fiStream.h"
    "rage/fiStream.cpp"
    "rage/fragType.h"
    "rage/fwFragmentDef.h"
    "rage/fwFsm.h"
    "rage/fwHelpers.h"
    "rage/fwTypes.h"
    "rage/grcTexture.h"
    "rage/grmShaderGroup.cpp"
    "rage/grmShaderGroup.h"
    "rage/pgDictionary.h"
    "rage/Pool.h"
    "rage/Streaming.h"
    "rage/TlsManager.h"
    "rage/atHashString.h"
    "rage/Vec3V.h"

    "rage_hook/grcore/rageRender.h"
    "rage_hook/grcore/rageDX11.h"
    "rage_hook/grcore/rageGrc.h" 
    "rage_hook/rageWin32.h"
    "rage_hook/rageFileInterface.h"
    "rage_hook/rageGrm.h" 
    "rage_hook/rageStreaming.h"
    "rage_hook/rageFwTimer.h" 
    "rage_hook/rageControls.h" 
    "rage_hook/rageHandlingHacks.h" 
    "rage_hook/rageHandling.h"

    "CrashHandler.h" "rage/fwModelId.h" "rage/CModelInfo.h" "../vendor/imgui-1.89.1/imgui_impl_win32.cpp" "../vendor/imgui-1.89.1/imgui_impl_win32.h")

# Boost Libary
set(BOOST_ROOT $ENV{Boost_ROOT})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
target_include_directories(rageAm PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(rageAm PUBLIC
    "${CMAKE_SOURCE_DIR}/vendor/minhook-1.3.3/lib/libMinHook-x64-v141-md.lib" 
    # "${CMAKE_SOURCE_DIR}/vendor/scripthook/lib/ScriptHookV.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/directxtk/lib/DirectXTK.lib" 
    "${CMAKE_SOURCE_DIR}/vendor/backtrace/libbacktrace.a" 
    d3d11.lib
    imgui
    ${Boost_LIBRARIES}
)

set_property(TARGET rageAm PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")

# Eject DLL and PDB (if stack trace was used) if already loaded
add_custom_command(TARGET ${PROJECT_NAME} 
    PRE_BUILD
    COMMAND 
    
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe 
    -fe rageAm.dll 
)

add_custom_command(TARGET ${PROJECT_NAME} 
    PRE_BUILD
    COMMAND 
    
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe 
    -pdb ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/rageAm.pdb
)

# Copy to temp directory, otherwise we can't compile DLL because its used by GTA.

# Use robocopy with repeating because sometimes copy fails... 
# || cmd /c "exit /b 0" bit is for making return code 0 (robocopy returns no 0 as success)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND 
    robocopy
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src
    ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp
    rageAm.dll
    rageAm.pdb
    /R:5
    /W:1
    /is /it
    /np /nfl /NJH /NDL
    || cmd /c "exit /b 0"
)

OPTION (INJECT_DLL "Inject DLL" ON)
OPTION (WAIT_FOR_GAME "Wait for game" ON)

# Inject DLL
if(INJECT_DLL)
    unset(INJECT_DLL CACHE)
    if(WAIT_FOR_GAME)
        # Unset it right after because build can be canceled in any moment
        unset(WAIT_FOR_GAME CACHE)
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
   
            ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe 
            -w ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp/rageAm.dll 
        )
    else()
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
   
            ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/injector/rageInjector.exe 
            -i ${CMAKE_SOURCE_DIR}/out/build/${CMAKE_PRESET_NAME}/src/temp/rageAm.dll 
        )
    endif(WAIT_FOR_GAME)
endif(INJECT_DLL)
        
unset(WAIT_FOR_GAME CACHE)
unset(INJECT_DLL CACHE)

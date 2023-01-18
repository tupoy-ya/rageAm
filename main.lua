require 'vendor'

function default_config()
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	cppdialect "C++20"
	vectorextensions "SSE2" -- seems to be ignored
	architecture "x64"
	flags { "MultiProcessorCompile" }

	symbols "On"
	filter "configurations:Debug"
		defines { "DEBUG" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Full"
		intrinsics "On"

	filter {}
end

function quote_path(path)
	return '\"' .. path .. '\"'
end

-- Adds launcher cmd command to eject DLL on pre build and inject it after build
function add_launcher_events(build_dir)
	-- Make sure all paths are quoted!
	build_dir = os.realpath(build_dir)
	
	local scylla = quote_path(os.realpath("tools/scyllahide.dll"))
	local launcher = quote_path(build_dir .. "Launcher.exe")
	local rageAm = quote_path(build_dir .. "rageAm.dll")

	local base_command = launcher .. " -exe GTA5.exe -dll " .. rageAm .. " -scylla " .. scylla
	prebuildcommands { base_command .. " -unload" }
	postbuildcommands { base_command .. " -load" }
end

function setup_launcher_events()
	filter "configurations:Debug"
		add_launcher_events "bin/Debug/"
	filter "configurations:Release"
		add_launcher_events "bin/Release/"
	filter{}
end

workspace "rageAm"
	configurations { "Debug", "Release" }

project "Launcher"
	kind "ConsoleApp"
	default_config()
	
	files { "launcher/**.h", "launcher/**.cpp" }

project "rageAm"
	kind "SharedLib"
	-- kind "ConsoleApp"
	-- defines { "RAGE_STANDALONE" }
	default_config()
	
	files { "src/**.h", "src/**.cpp", "src/**.hint", "src/**.natvis" }
	includedirs { "src", "src/common", "src/memory", "src/rage" }

	dofile("config.lua")

	setup_launcher_events()

	defines { "IMGUI_USER_CONFIG=" .. "\"" .. (os.realpath("src/imgui_rage/ImGuiConfigRage.h")) .. "\"" }

	include_vendors {
		"imgui",
		"boost",
		"tinyxml2",
		"minhook",
		"backward-cpp",
		"directxtex",
		"directxtk",
		"freetype",
		"zlib",
	}
	links { "Comctl32.lib" } -- TaskDialog

	filter "files:**.natvis"
		buildaction "Natvis"
	filter{}

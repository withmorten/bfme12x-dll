function setdebugpath (gamepath, exename, args)
	debugdir (path.translate(gamepath))
	debugcommand (path.translate(gamepath) .. "/" .. exename)
	debugargs (args)
end

function addcopypath (gamepath)
	postbuildcommands { '{COPYFILE} "%{cfg.buildtarget.abspath}" "' .. path.translate(gamepath) .. "/" .. '%{cfg.buildtarget.name}"' }
	postbuildcommands { '{COPYFILE} "%{cfg.buildtarget.directory}/%{cfg.buildtarget.basename}.pdb" "' .. path.translate(gamepath) .. "/" .. '%{cfg.buildtarget.basename}.pdb"' }
end

workspace "bfme12x"
	configurations
	{
		"Debug",
		"Release",
	}

	location "build"

project "bfme12x"
	defines { "DLL_NAME=\"" .. "bfme12x" .. "\"" }

	setdebugpath ("C:\\Games\\The Rise of the Witch-King", "RTS.exe", "-xres 800 -yres 600 -win -preferlocalfiles -quickstart -nomusic")
	addcopypath ("C:\\Games\\The Battle for Middle-Earth")
	addcopypath ("C:\\Games\\The Battle for Middle-Earth II")
	addcopypath ("C:\\Games\\The Rise of the Witch-King")
	addcopypath ("G:\\sharedwithtammo\\b4me\\___mod")
	addcopypath ("G:\\sharedwithtammo\\b4me2ep1_stuff\\___mod")

	kind "SharedLib"
	language "C++"
	targetname "bfme12x"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"

	files { "../../hacks/patcher/patcher.cpp" }
	files { "../../hacks/patcher/patcher.h" }
	includedirs { "../../hacks/patcher" }

	files { "../../hackloader/src/util.cpp" }
	files { "../../hackloader/src/util.h" }
	includedirs { "../../hackloader/src" }

	files { "src/asm.asm" }
	files { "src/*.cpp" }
	files { "src/*.h" }

	characterset ("MBCS")
	toolset ("v141_xp")
	links { "legacy_stdio_definitions", "winmm" }
	staticruntime "on"
	defines { "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		flags { "LinkTimeOptimization" }

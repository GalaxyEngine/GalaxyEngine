add_rules("mode.release", "mode.debug", "mode.gamedbg", "mode.game")
add_rules("plugin.vsxmake.autoupdate")

-- Runtime mode
if is_plat("windows") then
    set_runtimes((is_mode("debug") or is_mode("gamedbg")) and "MDd" or "MD")
end

set_allowedmodes("debug", "release", "gamedbg", "game")
set_defaultmode("debug")

local isEditor = is_mode("debug") or is_mode("release")
local isDebug = is_mode("debug") or is_mode("gamedbg")

if (isDebug) then
    add_defines("_DEBUG")
end

if (is_mode("gamedbg")) then
    add_defines("GAME_DEBUG")
    set_symbols("debug")
elseif (is_mode("game")) then
    add_defines("GAME_RELEASE")
    set_optimize("fastest")
    set_symbols("none")
    set_strip("all")
end

-- Modes
rule("mode.gamedbg")
rule_end()

rule("mode.game")
rule_end()

if (isEditor) then
    add_defines("WITH_EDITOR")
else
    add_defines("WITH_GAME")
end

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")

-- Packages
add_requires("galaxymath")
add_requires("cpp_serializer")
add_requires("galaxyscript v1.1-galaxyengine")
add_requires("imgui v1.90.7-docking", { configs = { opengl3 = true, glfw = true }})
add_requires("glad", {configs = { debug = isDebug, extensions = "GL_KHR_debug"}})
add_requires("stb")
add_requires("nativefiledialog-extended")
add_requires("openfbx")
add_requires("miniaudio")
add_requires("joltphysics", {configs = { debug = isDebug}})

-- enable features 
add_defines("ENABLE_MULTI_THREAD")

set_languages("c++20")

set_rundir("GalaxyCore")
set_targetdir("GalaxyCore")

-- diable warnings
add_cxflags("/wd4251", {tools = "cl"}) -- class needs to have dll-interface to be used by clients of class
add_cxflags("-Wall")            -- Enable all commonly used warning flags

target("GalaxyEngine")
    set_symbols("debug")
    set_kind("shared")

    -- set include dirs
    add_includedirs("GalaxyEngine/include")

    add_defines("GALAXY_EXPORTS")
    
    if (is_plat("windows", "msvc")) then 
        add_cxflags("/permissive")
        add_links("Advapi32")
        add_syslinks("opengl32")
    elseif (is_plat("linux")) then 
        add_cflags("-fPIC")
        print("Compile on Linux")
    end

    add_headerfiles("GalaxyEngine/include/**.h");
    add_headerfiles("GalaxyEngine/include/**.inl");
    add_files("GalaxyEngine/src/**.cpp")
    -- Includes --
    if (not isEditor) then
        remove_files("GalaxyEngine/include/Editor/**.h");
        remove_files("GalaxyEngine/include/Editor/**.inl");
        remove_files("GalaxyEngine/src/Editor/**.cpp")
    end
    set_pcxxheader("GalaxyEngine/include/pch.h")
    
    -- Packages --
    add_packages("galaxymath")
    add_packages("galaxyscript")
    add_packages("cpp_serializer")
    add_packages("glfw")
    add_packages("imgui")
    add_packages("glad")
    add_packages("stb")
    add_packages("nativefiledialog-extended")
    add_packages("openfbx")
    add_packages("miniaudio")
    add_packages("joltphysics")
    if (is_plat("mingw")) then 
        set_prefixname("")
    end 
target_end()

target("GalaxyCore")
    set_default(true)
    set_kind("binary")
    add_deps("GalaxyEngine")
    add_files("GalaxyCore/**.cpp")
    add_includedirs("GalaxyEngine/include")
    
    -- Packages
    add_packages("galaxymath")
    add_packages("imgui")
target_end()

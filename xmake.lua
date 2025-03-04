add_rules("mode.release", "mode.debug", "mode.gamedbg", "mode.game")
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" } )
add_rules("plugin.vsxmake.autoupdate")

-- Define physics API selection
option("physics_api")
    set_default("custom") -- Default physics API
    set_showmenu(true)
    set_description("Select Physics API (physx, jolt, custom)")
option_end()

-- Runtime mode configuration
if is_plat("windows") then
    set_runtimes((is_mode("debug") or is_mode("gamedbg")) and "MDd" or "MD")
end

set_allowedmodes("debug", "release", "gamedbg", "game")
set_defaultmode("debug")

local isEditor = is_mode("debug") or is_mode("release")
local isDebug = is_mode("debug") or is_mode("gamedbg")

if isDebug then
    add_defines("_DEBUG")
else
    if is_plat("windows") or is_plat("mingw") then
        add_ldflags("-subsystem:windows")
    end
    add_defines("NDEBUG")
end

if is_mode("gamedbg") then
    set_symbols("debug")
elseif is_mode("game") then
    set_optimize("fastest")
    set_symbols("none")
    set_strip("all")
end

-- Define additional modes
rule("mode.gamedbg")
rule_end()

rule("mode.game")
rule_end()

if isEditor then
    add_defines("WITH_EDITOR")
else
    add_defines("WITH_GAME")
end

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")

-- Required Packages
add_requires("galaxymath", "cpp_serializer", "galaxyscript v1.2-galaxyengine")
add_requires("galaxyheadertool 1.0-galaxyengine", { configs = { destdir = "GalaxyCore" } })
add_requires("imgui v1.90.7-docking", { configs = { opengl3 = true, glfw = true }})
add_requires("glad", { configs = { debug = isDebug, extensions = "GL_KHR_debug" }})
add_requires("stb", "nativefiledialog-extended", "openfbx", "miniaudio")

-- Detect and configure physics API
local use_physx = false
local use_jolt = false
local use_custom_physics = false

-- Retrieve and process selected physics APIs
local selected_apis = get_config("physics_api") or "custom"
local apis = {}
for api in selected_apis:gmatch("[^,]+") do
    table.insert(apis, api:trim())
end

-- Handle the installation of the appropriate packages based on the selected APIs
for _, api in ipairs(apis) do
    if api == "physx" then
        add_requires("vcpkg::physx")
        use_physx = true
    elseif api == "jolt" then
        add_requires("joltphysics")
        use_jolt = true
    elseif api == "custom" then
        use_custom_physics = true
    else
        print("Unknown Physics API: " .. api)
    end
end


-- Enable features
add_defines("ENABLE_MULTI_THREAD")

set_languages("c++20")
set_rundir("GalaxyCore")

-- Flags
add_cxflags("/wd4251", { tools = "cl" }) -- Disable "class needs to have dll-interface" warning
add_cxflags("-Wall") -- Enable all common warnings

-- Engine target
target("GalaxyEngine")
    set_symbols("debug")
    set_kind("shared")
    set_values("config.physics_api", physics_api)
    add_includedirs("GalaxyEngine/include")
    add_defines("GALAXY_EXPORTS")

    if is_plat("windows", "msvc") then
        add_cxflags("/permissive")
        add_links("Advapi32")
        add_syslinks("opengl32")
    elseif is_plat("linux") then
        add_cflags("-fPIC")
    end

    add_headerfiles("GalaxyEngine/include/**.h", "GalaxyEngine/include/**.inl")
    add_files("GalaxyEngine/src/**.cpp")

    if not isEditor then
        remove_files("GalaxyEngine/include/Editor/**.h", "GalaxyEngine/include/Editor/**.inl", "GalaxyEngine/src/Editor/**.cpp")
    end

    set_pcxxheader("GalaxyEngine/include/pch.h")
    add_packages("galaxymath", "galaxyscript", "cpp_serializer", "glfw", "imgui", "glad", "stb", "nativefiledialog-extended", "openfbx", "miniaudio")

    -- Physics API specific configurations
    if use_physx then
        add_packages("physx")
        add_defines("USE_PHYSX")
    else
        --remove_files("GalaxyEngine/include/Wrapper/PhysicAPI/PhysXPhysics.h", "GalaxyEngine/src/Wrapper/PhysicAPI/PhysXPhysics.cpp")
    end
    if use_jolt then
        add_packages("joltphysics")
        add_defines("USE_JOLT")
    else
        remove_files("GalaxyEngine/include/Wrapper/PhysicAPI/JoltPhysics.h", "GalaxyEngine/src/Wrapper/PhysicAPI/JoltPhysics.cpp")
    end
    if use_custom_physics then
        add_defines("USE_CUSTOM_PHYSICS")
    else
        remove_files("GalaxyEngine/include/Wrapper/PhysicAPI/CustomPhysics.h", "GalaxyEngine/src/Wrapper/PhysicAPI/CustomPhysics.cpp")
    end

    -- Print selected physics API after build
    after_build(function (target)
        print("Physics APIs selected: ")
        for k,v in pairs(apis) do
            print('['..(k-1)..']: '..v)
        end
    end)

    add_installfiles("GalaxyCore/imgui.ini", {prefixdir = "bin/"})
    add_installfiles("GalaxyCore/GalaxyHeaderTool.*", {prefixdir = "bin/"})

    -- Copy additional files/folders upon installation
    after_install(function (target)
        os.cp("GalaxyCore/CoreResources", target:installdir() .. "/bin")
    end)
target_end()

-- Core application target
target("GalaxyCore")
    set_default(true)
    set_kind("binary")
    add_deps("GalaxyEngine")
    add_files("GalaxyCore/**.cpp")
    add_includedirs("GalaxyEngine/include")
    add_packages("galaxymath", "imgui")
    add_files("GalaxyCore/CoreResources/icons/Editor.rc") -- Add this line to include the resource file
target_end()
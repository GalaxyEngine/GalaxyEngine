add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

add_requires("imgui v1.89.6-docking", { configs = { glfw_opengl3 = true } })
add_requires("glew")

set_languages("c++20")


if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

if is_plat("windows") then
    add_links("opengl32")
elseif is_plat("linux") then
    add_links("GL")
elseif is_plat("macosx") then
    add_frameworks("OpenGL")
end

function includeDir(path)
    add_headerfiles("GalaxyEngine/include/" .. path .. "/*.h")
    add_files("GalaxyEngine/src/" .. path .. "/*.cpp")
end

target("GalaxyEngine")
    set_kind("shared")
    set_symbols("debug")

    -- set include dirs
    add_includedirs("GalaxyEngine/include")

    add_defines("GALAXY_EXPORTS")

    includeDir("")
    includeDir("Wrapper")
    includeDir("Core")
    includeDir("Debug")
    includeDir("Resource")

    add_packages("glfw")
    add_packages("imgui")
    add_packages("glew")
target_end()

target("GalaxyCore")
    set_default(true)
    set_kind("binary")
    add_deps("GalaxyEngine")
    add_files("GalaxyCore/main.cpp")
    add_includedirs("GalaxyEngine/include")
target_end()
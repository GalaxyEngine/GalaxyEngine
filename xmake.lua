add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

add_requires("glfw 3.3.4", { system = false })
add_requires("imgui v1.89.6-docking", { configs = { glfw_opengl3 = true } })
add_requires("glew")

set_project("GalaxyEngine")

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
    add_headerfiles("include/" .. path .. "/*.h")
    add_files("src/" .. path .. "/*.cpp")
end

target("GalaxyEngine")
    set_kind("binary")
    set_pcxxheader("include/pch.h")
    set_symbols("debug")

    -- set include dirs
    add_includedirs("include")

    includeDir("")
    includeDir("Wrapper")
    includeDir("Core")
    includeDir("Debug")
    includeDir("Resource")

    add_packages("glfw")
    add_packages("imgui")
    add_packages("glew")

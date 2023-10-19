#pragma once
#include <GalaxyAPI.h>

#ifdef _WIN32
#include <Windows.h>
#endif

// C/C++ Headers
#include <memory>
#include <stdio.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>

// Galaxy Headers
#include <Debug/Log.h>
#include <Maths/Maths.h>
#include <Wrapper/Renderer.h>
#include <Wrapper/GUI.h>
#include <Wrapper/Reflection.h>
#include <Utils/FileSystem.h>

// Libs Headers
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//#include <misc/cpp/imgui_stdlib.h>

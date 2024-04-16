#pragma once
#include <GalaxyAPI.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
#define IMGUI_IMPLEMENTATION
#include <galaxymath/Maths.h>

#include "Wrapper/Serializer.h"

#include "Debug/Log.h"
#include "Debug/TimeProfiler.h"
#include "Wrapper/Renderer.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Reflection.h"
#include "Utils/FileSystem.h"
#include "Utils/Type.h"

// Libs Headers
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//#include <misc/cpp/imgui_stdlib.h>

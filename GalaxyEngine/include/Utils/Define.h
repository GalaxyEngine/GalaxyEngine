#pragma once

// -- Shaders -- //
#define PICKING_PATH ENGINE_RESOURCE_FOLDER_NAME"/shaders/PickingShader/picking.frag"
#define OUTLINE_PATH ENGINE_RESOURCE_FOLDER_NAME"/shaders/PostProcess/Outline/outline.ppshader"
#define BILLBOARD_PATH ENGINE_RESOURCE_FOLDER_NAME "/shaders/BillboardShader/billboard.shader"
#define GRID_PATH ENGINE_RESOURCE_FOLDER_NAME"/shaders/GridShader/grid.shader"
#define SKYBOX_SHADER_PATH ENGINE_RESOURCE_FOLDER_NAME"/shaders/Skybox/skybox.shader"

// -- Icons -- //
#define FOLDER_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/folder.png"
#define FILE_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/file.png"
#define LIGHT_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/light.png"
#define CAMERA_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/camera.png"
#define SCRIPT_CPP_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/script_cpp.png"
#define SCRIPT_H_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/script_h.png"
#define SHADER_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME "/icons/shader.png"

#define PLAYMODE_SCENE_PATH CACHE_PATH"playmode_scene.galaxy"
#define THUMBNAIL_PATH CACHE_PATH"thumbnail/"
#define PROJECT_THUMBNAIL_PATH CACHE_PATH"thumbnail/project.tmb"

// -- Meshes -- //
#define PLANE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Plane.obj:Plane"
#define CUBE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Cube.obj:Cube"
#define SPHERE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Sphere.obj:Sphere"

// -- Cubemaps -- //
#define CUBE_MAP_EXTENSION ".cubemap"
#define DEFAULT_SKYBOX_PATH ENGINE_RESOURCE_FOLDER_NAME"/cubemaps/default"CUBE_MAP_EXTENSION

// -- Paths -- //
#define ASSET_FOLDER_NAME "Assets"
#define ENGINE_RESOURCE_FOLDER_NAME "CoreResources"
#define EDITOR_SETTINGS_NAME "Editor.settings"
#define ENGINE_LOCATION_PATH CACHE_PATH"EngineLocation.txt"
#define CACHE_PATH "Cache/"
#define ENGINE_GENERATE_HEADER_PATH "Generate/Headers/"
#define PACKAGE_ASSEMBLY_NAME "Assembly"
#define PACKAGE_ASSEMBLY_DLL PACKAGE_ASSEMBLY_NAME DLL_EXT

// -- Misc -- //
#define NONE_RESOURCE "None"
#define MAX_LIGHT_NUMBER 8

#define INDEX_NONE 0xffffffff
#define UINT64_NULL 0xffffffffffffffffllu
#define UUID_NULL UINT64_NULL

// -- Colors -- //
#define BUTTON_RED Vec4f(0.8f, 0.15f, 0.1f, 1.f)

#define UNUSED(x) (void)(x)

//OS Define
#ifdef _WIN32
#define DLL_EXT ".dll"
#define BIN_EXT ".exe"
#elif defined(__linux__)
#define DLL_EXT ".so"
#define BIN_EXT ""
#elif defined(__APPLE__)
#define DLL_EXT ".dylib"
#define BIN_EXT ""
#endif
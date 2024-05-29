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

#define CACHE_PATH "Cache/"
#define PLAYMODE_SCENE_PATH CACHE_PATH"playmode_scene.galaxy"
#define THUMBNAIL_PATH CACHE_PATH"thumbnail/"
#define PROJECT_THUMBNAIL_PATH CACHE_PATH"thumbnail/project.tmb"

// -- Meshes -- //
#define PLANE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Plane.obj:Plane"
#define CUBE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Cube.obj:Cube"
#define SPHERE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Sphere.obj:Sphere"

// -- Cubemaps -- //
#define DEFAULT_SKYBOX_PATH ENGINE_RESOURCE_FOLDER_NAME"/cubemaps/default.cubemap"

// -- Paths -- //
#define ASSET_FOLDER_NAME "Assets"
#define ENGINE_RESOURCE_FOLDER_NAME "CoreResources"

// -- Misc -- //
#define NONE_RESOURCE "None"
#define MAX_LIGHT_NUMBER 8

#define INDEX_NONE 0xffffffff
#define UINT64_NULL 0xffffffffffffffffllu
#define UUID_NULL UINT64_NULL

// -- Colors -- //
#define BUTTON_RED Vec4f(0.8f, 0.15f, 0.1f, 1.f)

#define UNUSED(x) (void)(x)
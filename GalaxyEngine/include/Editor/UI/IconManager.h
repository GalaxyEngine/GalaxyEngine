#pragma once
#include "GalaxyAPI.h"

namespace GALAXY
{
    namespace Editor
    {
        namespace UI
        {
            class IconManager
            {
            public:
                static void Initialize();
                
                static ImTextureID CameraIcon;
                static ImTextureID LightIcon;
                static ImTextureID InfoIcon;
                static ImTextureID WarningIcon;
                static ImTextureID ErrorIcon;
                static ImTextureID SettingsIcon;
                static ImTextureID MenuIcon;
                static ImTextureID AddIcon;
                static ImTextureID RemoveIcon;
                static ImTextureID SearchIcon;
            };
        }
    }
}

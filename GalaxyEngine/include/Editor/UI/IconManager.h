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
                
                static uint32_t InfoIcon;
                static uint32_t WarningIcon;
                static uint32_t ErrorIcon;
                static uint32_t SettingsIcon;
                static uint32_t MenuIcon;
                static uint32_t AddIcon;
                static uint32_t RemoveIcon;
                static uint32_t SearchIcon;

                static uint32_t CameraComponentIcon;
                static uint32_t LightComponentIcon;
                static uint32_t TransformComponentIcon;
                static uint32_t MeshComponentIcon;
                static uint32_t ScriptComponentIcon;
                
            };
        }
    }
}

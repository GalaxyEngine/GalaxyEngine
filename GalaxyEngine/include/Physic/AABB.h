#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Physic 
    {
        struct GALAXY_API AABB
        {
            Vec3f Min;
            Vec3f Max;

            Vec3f GetCenter() const { return (Min + Max) * 0.5f; }
            Vec3f GetExtents() const { return (Max - Min) * 0.5f; }

            AABB() {}
            AABB(Vec3f min, Vec3f max) : Min(min), Max(max) {}
            AABB(Vec3f center, Vec3f extents, bool isHalfExtents) : Min(center - extents), Max(center + extents) {}
        };
    }
}

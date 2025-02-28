#include "pch.h"
#include "Physic/Ray.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    bool Physic::Raycast(const Ray& ray, float maxDistance, RaycastHit& hit)
    {
        return Raycast(ray.origin, ray.direction, maxDistance, hit);
    }

    bool Physic::Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance, RaycastHit& hit)
    {
        if (auto instance = Wrapper::PhysicsWrapper::GetInstance())
        {
            return instance->Raycast(origin, direction, maxDistance, hit);
        }
        return false;
    }
}

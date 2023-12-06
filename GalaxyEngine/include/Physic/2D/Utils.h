#pragma once
#include "GalaxyAPI.h"

#include "Physic/2D/Rect.h"

namespace GALAXY 
{
	namespace Physic2D
	{
		namespace Utils
		{
			template<typename T, typename U>
			inline bool IsPointInsideRect(const Vec2<T>& point, const Rect<U>& rectangle);
		}
	}
}

#include "Physic/2D/Utils.inl" 

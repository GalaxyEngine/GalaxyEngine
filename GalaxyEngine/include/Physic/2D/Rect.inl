#pragma once
#include "Physic/2D/Rect.h"
namespace GALAXY
{
	namespace Physic2D {
		template<typename T>
		template<typename U>
		inline bool Rect<T>::IsPointInside(const Vec2<U>& point) const
		{
			return point.x >= min.x && point.x <= max.x
				&& point.y >= min.y && point.y <= max.y;
		}
	}
}

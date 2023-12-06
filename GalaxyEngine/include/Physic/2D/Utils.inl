#pragma once
#include "Core/Input.h"
#include "Physic/2D/Utils.h"
namespace GALAXY
{

	template<typename T, typename U>
	bool Physic2D::Utils::IsPointInsideRect(const Vec2<T>& point,const Rect<U>& rectangle)
	{
		return rectangle.IsPointInside(point);
	}
}

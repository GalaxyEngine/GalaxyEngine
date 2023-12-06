#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Physic2D {
		template<typename T>
		struct Rect
		{
			Vec2<T> max;
			Vec2<T> min;

			template<typename U>
			inline bool IsPointInside(const Vec2<U>& point) const;

			void Print() const;
		};

		typedef Rect<float> Rectf;
		typedef Rect<int> Recti;
		typedef Rect<double> Rectd;
	}
}
#include "Physic/2D/Rect.inl" 

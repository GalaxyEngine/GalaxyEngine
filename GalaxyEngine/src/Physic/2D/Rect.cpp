#include "pch.h"
#include "Physic/2D/Rect.h"
namespace GALAXY 
{
	namespace Physic2D
	{
		template<typename T>
		void Rect<T>::Print() const { }

		template<>
		void Recti::Print() const {
			PrintLog("Recti: %d, %d, %d, %d", min.x, min.y, max.x, max.y);
		}

		template<>
		void Rectf::Print() const {
			PrintLog("Rectf: %f, %f, %f, %f", min.x, min.y, max.x, max.y);
		}

		template<>
		void Rectd::Print() const {
			PrintLog("Rectd: %f, %f, %f, %f", min.x, min.y, max.x, max.y);
		}
	}
}

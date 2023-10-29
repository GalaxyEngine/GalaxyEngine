#pragma once
#include "GalaxyAPI.h"

#include <functional>
namespace GALAXY 
{
	namespace Editor
	{
		class Action
		{
		public:
			inline Action(const std::function<void()>& doFunc, const std::function<void()>& undoFunc) : doAction(doFunc), undoAction(undoFunc) {}
			inline Action& operator=(const Action& other) = default;
			inline Action(const Action&) = default;
			inline Action(Action&&) noexcept = default;
			inline virtual ~Action() {}

			inline void Do() { doAction(); }
			inline void Undo() { undoAction(); }

		private:
			std::function<void()> doAction;
			std::function<void()> undoAction;

		};
	}
}
#include "Editor/Action.inl" 

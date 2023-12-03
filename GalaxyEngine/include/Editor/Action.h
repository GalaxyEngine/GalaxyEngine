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
			inline Action() {}
			inline Action(const std::function<void()>& doFunc, const std::function<void()>& undoFunc) : doAction(doFunc), undoAction(undoFunc) {}
			inline Action& operator=(const Action& other) = default;
			inline Action(const Action&) = default;
			inline Action(Action&&) noexcept = default;
			inline virtual ~Action() {}

			inline void Bind(const std::function<void()>& doFunc, const std::function<void()>& undoFunc)
			{
				doAction = doFunc;
				undoAction = undoFunc;
			}

			inline void Do() const { doAction(); }
			inline void Undo() const { undoAction(); }

		private:
			std::function<void()> doAction;
			std::function<void()> undoAction;

		};
	}
}
#include "Editor/Action.inl" 

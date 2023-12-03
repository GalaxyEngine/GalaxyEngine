#pragma once
#include "GalaxyAPI.h"

#include "Editor/Action.h"

#include <queue>

namespace GALAXY 
{
	class Action;
	namespace Editor
	{
		class ActionManager
		{
		public:
			inline ActionManager() {}

			inline void AddAction(const Action& action) { undoStack.push_back(action); }
			inline bool CanUndo() const {
				return !undoStack.empty();
			}

			inline bool CanRedo() const {
				return !redoStack.empty();
			}

			void Update();

			inline void Undo() {
				if (CanUndo()) {
					const Action action = undoStack.back();
					undoStack.pop_back();
					action.Undo();
					redoStack.push_back(action);
				}
			}

			inline void Redo() {
				if (CanRedo()) {
					const Action action = redoStack.back();
					redoStack.pop_back();
					action.Do();
					undoStack.push_back(action);
				}
			}

		private:
			std::vector<Action> undoStack;
			std::vector<Action> redoStack;
		};
	}
}
#include "Editor/ActionManager.inl" 

#pragma once
#include "Component/ScriptComponent.h"
#include "Wrapper/GUI.h"
namespace GALAXY 
{
	const Vec2f buttonSize = Vec2f(24);
	template<typename T> 
	inline void Component::ScriptComponent::DisplayAndManageVariable(const std::pair<std::string, Scripting::VariableData>& variable)
	{
		if (!variable.second.isAList) {
			DisplayVariableT(variable, GetVariable<T>(variable.first));
		}
		else {
			if (std::vector<T>* list = GetVariable<std::vector<T>>(variable.first)) {
				Wrapper::GUI::SetNextItemOpen();
				if (Wrapper::GUI::TreeNode(variable.first.c_str())) {
					for (size_t i = 0; i < list->size(); i++) {
						Wrapper::GUI::PushID((int)i);
						T value = (*list)[i];
						DisplayVariableT(variable, &value);
						(*list)[i] = value;
						Wrapper::GUI::PopID();
					}
					if (Wrapper::GUI::Button("+", buttonSize)) {
						list->push_back({});
					}
					Wrapper::GUI::SameLine();
					if (Wrapper::GUI::Button("-", buttonSize) && !list->empty()) {
						list->pop_back();
					}
					Wrapper::GUI::TreePop();
				}
			}
		}
	}
}

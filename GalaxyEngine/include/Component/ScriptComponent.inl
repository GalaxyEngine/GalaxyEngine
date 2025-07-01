#pragma once
#include "Component/ScriptComponent.h"
#ifdef WITH_EDITOR
#include "Wrapper/UI.h"
#endif
namespace GALAXY 
{
	/*
	const Vec2f buttonSize = Vec2f(24);
	template<typename T> 
	inline void Component::ScriptComponent::DisplayAndManageVariable(const std::pair<std::string, Scripting::VariableData>& variable)
	{
		if (!variable.second.isAList) {
			DisplayVariableT(variable, GetVariable<T>(variable.first));
		}
		else {
			if (std::vector<T>* list = GetVariable<std::vector<T>>(variable.first)) {
				Wrapper::UI::SetNextItemOpen();
				if (Wrapper::UI::TreeNode(variable.first.c_str())) {
					for (size_t i = 0; i < list->size(); i++) {
						Wrapper::UI::PushID(static_cast<int>(i));
						T value = (*list)[i];
						DisplayVariableT(variable, &value);
						(*list)[i] = value;
						Wrapper::UI::PopID();
					}
					if (Wrapper::UI::Button("+", buttonSize)) {
						list->push_back({});
					}
					Wrapper::UI::SameLine();
					if (Wrapper::UI::Button("-", buttonSize) && !list->empty()) {
						list->pop_back();
					}
					Wrapper::UI::TreePop();
				}
			}
		}
	}
	*/
}

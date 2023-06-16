#pragma once
#include "GalaxyAPI.h"
#include <rttr/registration>

using namespace rttr;
namespace GALAXY::Wrapper::Reflection
{
	// Print the values of a given instance
	// Recursive function to print the properties of custom classes
	void PrintClass(const rttr::variant& object, const std::string& indent = "");

	template<typename T>
	void ShowInspectorClass(T* object);
}

template<typename T>
void GALAXY::Wrapper::Reflection::ShowInspectorClass(T* object)
{
	if (!object)
		return;

	rttr::variant variant = *object;
	rttr::type objectType = variant.get_type();

	// Iterate over the properties
	for (const auto& property : objectType.get_properties()) {
		const rttr::type& propertyType = property.get_type();
		if (propertyType.get_metadata("Hidden").to_bool())
			continue;

		if (propertyType == rttr::type::get<float>()) {
			float value = property.get_value(variant).get_value<float>();
			if (ImGui::DragFloat(property.get_name().to_string().c_str(), &value)) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<int>()) {
			int value = property.get_value(*object).get_value<int>();
			if (ImGui::DragInt(property.get_name().to_string().c_str(), &value)) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<bool>()) {
			bool value = property.get_value(*object).get_value<bool>();
			if (ImGui::Checkbox(property.get_name().to_string().c_str(), &value)) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<Vec3f>()) {
			Vec3f value = property.get_value(*object).get_value<Vec3f>();
			if (ImGui::DragFloat3(property.get_name().to_string().c_str(), value.Data())) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<Vec3i>()) {
			Vec3i value = property.get_value(*object).get_value<Vec3i>();
			if (ImGui::DragInt3(property.get_name().to_string().c_str(), value.Data())) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<Vec4f>()) {
			Vec4f value = property.get_value(*object).get_value<Vec4f>();
			if (ImGui::ColorEdit4(property.get_name().to_string().c_str(), value.Data())) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
		else if (propertyType == rttr::type::get<std::string>()) {
			std::string value = property.get_value(*object).get_value<std::string>();
			if (ImGui::InputText(property.get_name().to_string().c_str(), &value)) {
				property.set_value(*object, value); // Set the updated value back to the original object
			}
		}
	}
}



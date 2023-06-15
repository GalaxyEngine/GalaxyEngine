#include "pch.h"
#include "Wrapper/Reflection.h"

//Classes Include
#include "Component/Transform.h"

#ifdef ENABLE_REFLECTION
#define PROPRETY_HIDDEN (metadata("Hidden", true))


RTTR_REGISTRATION
{
	rttr::registration::class_<Component::Transform>("Transform")
		.property("LocalPosition", &Component::Transform::m_localPosition)
		.property("LocalRotation", &Component::Transform::m_localRotation) PROPRETY_HIDDEN
		.property("LocalEulerRotation", &Component::Transform::m_localEulerRotation)
		.property("LocalScale", &Component::Transform::m_localScale);


	registration::class_<Vec3i>("Vec3i")
		.constructor<>()
		.property("x", &Vec3i::x)
		.property("y", &Vec3i::y)
		.property("z", &Vec3i::z);

	registration::class_<Vec3f>("Vec3f")
		.constructor<>()
		.property("x", &Vec3f::x)
		.property("y", &Vec3f::y)
		.property("z", &Vec3f::z);

	registration::class_<Vec3d>("Vec3d")
		.constructor<>()
		.property("x", &Vec3d::x)
		.property("y", &Vec3d::y)
		.property("z", &Vec3d::z);

	registration::class_<Quat>("Quat")
		.constructor<>()
		.property("x", &Quat::x)
		.property("y", &Quat::y)
		.property("z", &Quat::z)
		.property("w", &Quat::w);
}
#endif

void GALAXY::Wrapper::Reflection::PrintClass(const rttr::variant& object, const std::string& indent /*= ""*/)
{
	rttr::type objectType = object.get_type();

	// Iterate over the properties
	for (const auto& property : objectType.get_properties()) {
		std::string propertyName = property.get_name().to_string();
		std::cout << indent << propertyName << ": ";

		rttr::variant propertyValue = property.get_value(object);

		if (property.get_type().is_class()) {
			std::cout << indent << propertyName << "\n";
			PrintClass(propertyValue, indent + "    ");
		}
		else {
			std::string propertyValueString = propertyValue.to_string();
			std::cout << propertyValueString << std::endl;
		}
	}
}
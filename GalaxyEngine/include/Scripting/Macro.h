#pragma once
#include "GalaxyAPI.h"

#define EXPORT_FUNC extern "C" __declspec(dllexport)
#define UCLASS(x) \
	class x;\
	EXPORT_FUNC void* Create##_##x() {return new x(); }

#define UPROPERTY(x, y) \
	EXPORT_FUNC void* Get_##x_##y(x* object) { return &object->y;} \
	EXPORT_FUNC void Set_##x_##y(x* object, void* variable){ object->y = *reinterpret_cast<decltype(object->y)*>(variable); }

#define GENERATED_BODY(x)\
	std::string GetComponentName() const override { return typeid(*this).name(); }

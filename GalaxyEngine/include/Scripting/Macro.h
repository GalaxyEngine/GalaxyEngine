#pragma once
#include "GalaxyAPI.h"
#ifdef _WIN32
#define EXPORT_FUNC extern "C" __declspec(dllexport)
#elif defined(__linux__)
#define EXPORT_FUNC extern "C"
#endif
#define CLASS(x) \
	EXPORT_FUNC Component::BaseComponent* Create##_##x() { return new x(); }

#define PROPERTY(x, y) \
	EXPORT_FUNC void* Get_##x##_##y(x* object) { return &object->y;} \
	EXPORT_FUNC void Set_##x##_##y(x* object, void* variable){ object->y = *reinterpret_cast<decltype(object->y)*>(variable); }

#define GENERATED_BODY(x, y)\
public:\
	typedef y Super;\
	x() {}\
	x& operator=(const x& other) = default;\
	x(const x&) = default;\
	x(x&&) noexcept = default;\
	virtual ~x() {}\
	const char* GetComponentName() const override \
		{ \
			return #x;\
		}\
	virtual List<const char*> GetComponentNames() const override\
	{\
		auto vector = Super::GetComponentNames();\
		vector.insert(vector.end(), x::GetComponentName());\
		return vector;\
	}\
	virtual Shared<Component::BaseComponent> Clone() override {\
		return std::make_shared<x>(*dynamic_cast<x*>(this));\
	}\
private:

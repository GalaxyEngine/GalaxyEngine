#include "pch.h"
#include "Wrapper/Serializer.h"
#include "Core/UUID.h"

#include "Core/GameObject.h"
#include "Component/IComponent.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

// --------------------------------------- Serializer ---------------------------------------

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec2f& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec3f& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec4f& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec2i& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec3i& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec4i& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec2<uint64_t>& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec3<uint64_t>& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Vec4<uint64_t>& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Quat& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Mat4& value)
{
	const std::string stringValue = value.ToString();
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const Core::UUID& value)
{
	const std::string stringValue = std::to_string(value);
	*this << stringValue.c_str();
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(Core::GameObject* value)
{
	if (!value) {
		this->operator<<(-1);
		return *this;
	}

	this->operator<<(value->GetUUID());
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(Component::BaseComponent* value)
{
	std::string pairString;
	if (!value)
		pairString = std::to_string(-1) + ", " + std::to_string(-1);
	else
		pairString = std::to_string(value->GetGameObject()->GetUUID()) + ", " + std::to_string(value->GetIndex());
	this->operator<<(pairString);
	return *this;
}

template<> CppSer::Serializer& CppSer::Serializer::operator<<(const unsigned long& value)
{
	auto stringValue = std::to_string(value);
	*this << stringValue;
	return *this;
}
// --------------------------------------- Parser ---------------------------------------
template<>
uint32_t CppSer::StringSerializer::As() const
{
	return std::stoi(m_content);
}

template <>
Vec2f CppSer::StringSerializer::As() const
{
	return { m_content };
}

template <>
Vec2<uint64_t> CppSer::StringSerializer::As() const
{
	return { m_content };
}

template <>
Vec3f CppSer::StringSerializer::As() const
{
	return { m_content };
}

template <>
Vec4f CppSer::StringSerializer::As() const
{
	return { m_content };
}

template <>
Quat CppSer::StringSerializer::As() const
{
	return { m_content };
}

template <>
Weak<Core::GameObject> CppSer::StringSerializer::As() const
{
	const uint64_t index = As<uint64_t>();
	return Core::SceneHolder::GetCurrentScene()->GetWithUUID(index);
}

template <>
Component::ComponentID CppSer::StringSerializer::As() const
{
	uint64_t goIndex;
	uint32_t compIndex;
	std::stringstream os(m_content);
	char skipChar;
	os >> goIndex >> skipChar >> compIndex;

	return { goIndex, compIndex };
}

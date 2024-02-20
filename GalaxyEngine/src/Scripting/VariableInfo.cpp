#include "pch.h"
#include "Scripting/VariableInfo.h"

#include <ScriptEngine.h>
namespace GALAXY
{
	using namespace Scripting;
	std::unordered_map<std::string, VariableType> variableTypeMap =
	{
		{"bool", VariableType::Bool},
		{"int", VariableType::Int},
		{"float", VariableType::Float},
		{"double", VariableType::Double},
		{"string", VariableType::String},
		{"Vec2f", VariableType::Vector2f},
		{"Vec3f", VariableType::Vector3f},
		{"Vec4f", VariableType::Vector4f},
		{"Quat", VariableType::Quaternion},
	};

	VariableInfo::VariableInfo(const GS::Property& variable)
	{
		Initialize(variable);
	}

	void VariableInfo::Initialize(const GS::Property& variable)
	{
		this->args = variable.args;
		this->name = variable.name;
		this->typeName = variable.type;

		std::string variableType = variable.type;
		if (variableType.find("::") != std::string::npos)
		{
			variableType = variable.type.substr(variable.type.find_last_of("::") + 1);
			if (variableType.find("vector<") != std::string::npos)
			{
				this->isAList = true;
				variableType = variableType.substr(variableType.find_last_of("<") + 1);
				variableType = variableType.substr(0, variableType.find_last_of(">"));
			}
		}

		this->type = TypeNameToType(variableType);
	}

	void VariableInfo::BeginSerializeList(CppSer::Serializer& serializer, const std::string& name, size_t size)
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << size;
	}

	size_t VariableInfo::BeginDeserializeList(CppSer::Parser& parser, const std::string& name)
	{
		return parser[name].As<size_t>();
	}

	VariableType VariableInfo::TypeNameToType(const std::string& typeName)
	{
		if (variableTypeMap.contains(typeName))
			return variableTypeMap[typeName];
		return VariableType::Unknown;
	}

#pragma region DisplayValue
	template<typename T>
	void Scripting::VariableInfoT<T>::DisplayValue(const std::string& name, void* value)
	{
		ImGui::Text(name.c_str());
	}

	template<>
	void Scripting::VariableInfoT<bool>::DisplayValue(const std::string& name, void* value)
	{
		ImGui::Checkbox(name.c_str(), (bool*)value);
	}

	template<>
	void Scripting::VariableInfoT<int>::DisplayValue(const std::string& name, void* value)
	{
		ImGui::DragInt(name.c_str(), (int*)value);
	}

	template<>
	void Scripting::VariableInfoT<float>::DisplayValue(const std::string& name, void* value)
	{
		ImGui::DragFloat(name.c_str(), (float*)value, 0.01f);
	}

	template<>
	void Scripting::VariableInfoT<double>::DisplayValue(const std::string& name, void* value)
	{
		ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, value);
	}

	template<>
	void Scripting::VariableInfoT<std::string>::DisplayValue(const std::string& name, void* value)
	{
		Wrapper::GUI::InputText(name.c_str(), (std::string*)value);
	}

	template<>
	void Scripting::VariableInfoT<Vec2f>::DisplayValue(const std::string& name, void* value)
	{
		Vec3f v = *(Vec3f*)value;
		if (ImGui::DragFloat3(name.c_str(), &v.x, 0.01f))
		{
			*(Vec2f*)value = v;
		}
	}

	template<>
	void Scripting::VariableInfoT<Vec3f>::DisplayValue(const std::string& name, void* value)
	{
		Vec3f v = *(Vec3f*)value;
		if (ImGui::DragFloat3(name.c_str(), &v.x, 0.01f))
		{
			*(Vec3f*)value = v;
		}
	}

	template<>
	void Scripting::VariableInfoT<Vec4f>::DisplayValue(const std::string& name, void* value)
	{
		Vec4f v = *(Vec4f*)value;
		if (ImGui::ColorEdit4(name.c_str(), &v.x))
		{
			*(Vec4f*)value = v;
		}
	}

	template<>
	void Scripting::VariableInfoT<Quat>::DisplayValue(const std::string& name, void* value)
	{
		Quat v = *(Quat*)value;
		if (ImGui::DragFloat4(name.c_str(), &v.x, 0.01f))
		{
			*(Quat*)value = v;
		}
	}
#pragma endregion

#pragma region Serialize
	template<typename T>
	void Scripting::VariableInfoT<T>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		PrintError("Cannot serialize variable: %s", name.c_str());
	}

	template<>
	void Scripting::VariableInfoT<bool>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(bool*)value;
	}

	template<>
	void Scripting::VariableInfoT<int>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(int*)value;
	}

	template<>
	void Scripting::VariableInfoT<float>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(float*)value;
	}

	template<>
	void Scripting::VariableInfoT<double>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(double*)value;
	}

	template<>
	void Scripting::VariableInfoT<std::string>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(std::string*)value;
	}

	template<>
	void Scripting::VariableInfoT<Vec2f>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(Vec2f*)value;
	}

	template<>
	void Scripting::VariableInfoT<Vec3f>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(Vec3f*)value;
	}

	template<>
	void Scripting::VariableInfoT<Vec4f>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(Vec4f*)value;
	}

	template<>
	void Scripting::VariableInfoT<Quat>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << *(Quat*)value;
	}
#pragma endregion

#pragma region Deserialize
	template<typename T>
	void Scripting::VariableInfoT<T>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		PrintError("Cannot Deserialize variable: %s", name.c_str());
	}

	template<>
	void Scripting::VariableInfoT<bool>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(bool*)value = parser[name].As<bool>();
	}

	template<>
	void Scripting::VariableInfoT<int>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(int*)value = parser[name].As<int>();
	}

	template<>
	void Scripting::VariableInfoT<float>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(float*)value = parser[name].As<float>();
	}

	template<>
	void Scripting::VariableInfoT<double>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(double*)value = parser[name].As<double>();
	}

	template<>
	void Scripting::VariableInfoT<std::string>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(std::string*)value = parser[name].As<std::string>();
	}

	template<>
	void Scripting::VariableInfoT<Vec2f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(Vec2f*)value = parser[name].As<Vec2f>();
	}

	template<>
	void Scripting::VariableInfoT<Vec3f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(Vec3f*)value = parser[name].As<Vec3f>();
	}

	template<>
	void Scripting::VariableInfoT<Vec4f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(Vec4f*)value = parser[name].As<Vec4f>();
	}

	template<>
	void Scripting::VariableInfoT<Quat>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const
	{
		*(Quat*)value = parser[name].As<Quat>();
	}
#pragma endregion

}

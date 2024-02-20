#include "pch.h"
#include "Scripting/VariableInfo.h"

#include <ScriptEngine.h>

#include "Component/ComponentHolder.h"
#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

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
		{"GameObject", VariableType::GameObject},
		{"BaseComponent", VariableType::Component},
		{"ScriptComponent", VariableType::Component },
	};

	VariableInfo::VariableInfo(const GS::Property& variable)
	{
		Initialize(variable);
	}

	void VariableInfo::SanitizeType(std::string& typeName)
	{
		if (typeName.find("vector<") != std::string::npos)
		{
			typeName = typeName.substr(typeName.find_last_of("<") + 1);
			typeName = typeName.substr(0, typeName.find_last_of(">"));
		}
		if (typeName.find("*") != std::string::npos)
		{
			typeName = typeName.substr(0, typeName.find_first_of("*"));
		}
		if (typeName.find("::") != std::string::npos)
		{
			typeName = typeName.substr(typeName.find_last_of("::") + 1);
		}
	}

	void VariableInfo::Initialize(const GS::Property& variable)
	{
		this->args = variable.args;
		this->name = variable.name;
		this->basetypeName = variable.type;
		this->typeName = variable.type;

		std::string variableType = variable.type;
		if (variableType.find("vector<") != std::string::npos)
		{
			this->isAList = true;
			variableType = variableType.substr(variableType.find_last_of("<") + 1);
			variableType = variableType.substr(0, variableType.find_last_of(">"));
		}
		if (variableType.find("*") != std::string::npos)
		{
			variableType = variableType.substr(0, variableType.find_first_of("*"));
		}
		if (variableType.find("::") != std::string::npos)
		{
			variableType = variableType.substr(variableType.find_last_of("::") + 1);
		}
		this->typeName = variableType;

		this->type = TypeNameToType(variableType);
	}

	void VariableInfo::BeginSerializeList(CppSer::Serializer& serializer, const std::string& name, size_t size)
	{
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << size;
	}

	void VariableInfo::BeginSerialize(CppSer::Serializer& serializer, const std::string& name) const
	{
		serializer << CppSer::Pair::Key << name + " Type" << CppSer::Pair::Value << this->basetypeName;
	}

	size_t VariableInfo::BeginDeserializeList(CppSer::Parser& parser, const std::string& name)
	{
		return parser[name].As<size_t>();
	}

	bool VariableInfo::BeginDeserialize(CppSer::Parser& parser, const std::string& name) const
	{
		const auto baseTypeName = parser[name + " Type"].As<std::string>();
		return this->basetypeName == baseTypeName;
	}

	VariableType VariableInfo::TypeNameToType(const std::string& typeName)
	{
		if (variableTypeMap.contains(typeName))
			return variableTypeMap[typeName];

		for (auto& instanceComponent : Component::ComponentHolder::GetList())
		{
			if (instanceComponent->GetComponentName() == typeName)
				return VariableType::Component;
		}

		return VariableType::Unknown;
	}

	void VariableInfo::AfterLoad(Resource::Scene* scene) {
		if (gameObjectReloaders) {
			for (auto& gameObjectReloader : *gameObjectReloaders) {
				if (auto object = scene->GetWithUUID(gameObjectReloader.entityID).lock()) {
					auto gameObjectPtr = reinterpret_cast<Core::GameObject**>(gameObjectReloader.gameObjectPtr);
					*gameObjectPtr = object.get();
				}
			}
		}

		if (componentReloaders) {
			for (auto& componentReloader : *componentReloaders) {
				if (auto object = scene->GetWithUUID(componentReloader.entityID).lock()) {
					auto componentPtr = reinterpret_cast<Component::BaseComponent**>(componentReloader.componentPtr);
					if (auto component = object->GetComponentWithIndex(componentReloader.componentID).lock()) {
						*componentPtr = component.get();
					}
				}
			}
		}
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

	template<>
	void Scripting::VariableInfoT<Core::GameObject*>::DisplayValue(const std::string& name, void* value)
	{
		Core::GameObject** go = (Core::GameObject**)value;
		const Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x / 2.f, 0);
		ImGui::Button(*go ? (*go)->GetName().c_str() : "None", buttonSize);
		if (*go) {
			ImGui::SameLine();
			if (ImGui::Button("Reset", Vec2f(ImGui::GetContentRegionAvail().x, 0)))
				(*go) = nullptr;
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
				// Check if the payload data type matches
				std::vector<uint64_t> indices;
				if (payload->DataSize % sizeof(uint64_t) == 0)
				{
					uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
					const uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
					indices.assign(payloadData, payloadData + payloadSize);
				}
				// Get the gameobject with the first index
				*go = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithSceneGraphID(indices[0]).lock().get();
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("(GameObject)");
	}

	template<>
	void Scripting::VariableInfoT<Component::BaseComponent*>::DisplayValue(const std::string& name, void* value)
	{
		Component::BaseComponent** component = (Component::BaseComponent**)value;
		const Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x / 2.f, 0);
		ImGui::Button(*component ? (*component)->GetComponentName() : "None", buttonSize);
		if (*component) {
			ImGui::SameLine();
			if (ImGui::Button("Reset", Vec2f(ImGui::GetContentRegionAvail().x, 0)))
			{
				(*component) = nullptr;
			}
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
				// Check if the payload data type matches
				std::vector<uint64_t> indices;
				if (payload->DataSize % sizeof(uint64_t) == 0)
				{
					uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
					const uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
					indices.assign(payloadData, payloadData + payloadSize);
				}
				for (const uint64_t indice : indices)
				{
					// Get the gameobject with the indices
					std::weak_ptr<Core::GameObject> payloadGameObject;
					payloadGameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithSceneGraphID(indice);
					if (const auto component2 = payloadGameObject.lock()->GetComponentWithName(typeName))
					{
						// Get Component of the good type with the name
						(*component) = component2;
						return;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("(%s)", typeName.c_str());
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

	template<>
	void Scripting::VariableInfoT<Core::GameObject*>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		auto gameObject = *(Core::GameObject**)value;
		if (!gameObject)
			return;
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << gameObject->GetUUID();
	}

	template<>
	void Scripting::VariableInfoT<Component::BaseComponent*>::SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const
	{
		Component::BaseComponent* component = *(Component::BaseComponent**)value;
		if (!component)
			return;
		uint64_t componentID = component->GetIndex();
		uint64_t entityID = component->GetGameObject()->GetUUID();
		serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << Vec2<uint64_t>(entityID, componentID);
	}

#pragma endregion

#pragma region Deserialize
	template<typename T>
	void Scripting::VariableInfoT<T>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		PrintError("Cannot Deserialize variable: %s", name.c_str());
	}

	template<>
	void Scripting::VariableInfoT<bool>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(bool*)value = parser[name].As<bool>();
	}

	template<>
	void Scripting::VariableInfoT<int>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(int*)value = parser[name].As<int>();
	}

	template<>
	void Scripting::VariableInfoT<float>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(float*)value = parser[name].As<float>();
	}

	template<>
	void Scripting::VariableInfoT<double>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(double*)value = parser[name].As<double>();
	}

	template<>
	void Scripting::VariableInfoT<std::string>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(std::string*)value = parser[name].As<std::string>();
	}

	template<>
	void Scripting::VariableInfoT<Vec2f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(Vec2f*)value = parser[name].As<Vec2f>();
	}

	template<>
	void Scripting::VariableInfoT<Vec3f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(Vec3f*)value = parser[name].As<Vec3f>();
	}

	template<>
	void Scripting::VariableInfoT<Vec4f>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(Vec4f*)value = parser[name].As<Vec4f>();
	}

	template<>
	void Scripting::VariableInfoT<Quat>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		*(Quat*)value = parser[name].As<Quat>();
	}

	template<>
	void Scripting::VariableInfoT<Core::GameObject*>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		uint64_t entityID = parser[name].As<uint64_t>();
		auto object = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithUUID(entityID);
		if (!object.lock())
		{
			GameObjectReloader reloader;
			reloader.entityID = entityID;
			reloader.gameObjectPtr = value;
			if (!gameObjectReloaders.has_value())
				gameObjectReloaders = std::vector<GameObjectReloader>();
			gameObjectReloaders->push_back(reloader);
			return;
		}
		*(Core::GameObject**)value = object.lock().get();
	}

	template<>
	void Scripting::VariableInfoT<Component::BaseComponent*>::DeserializeT(CppSer::Parser& parser, const std::string& name, void* value)
	{
		Vec2<uint64_t> ids = parser[name].As<Vec2<uint64_t>>();
		auto object = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithUUID(ids.x);
		bool loaded = false;
		if (object.lock())
		{
			Component::BaseComponent* component = object.lock()->GetComponentWithIndex((uint32_t)ids.y).lock().get();
			if (component) {
				loaded = true;
				*(Component::BaseComponent**)value = component;
			}
		}

		if (!loaded)
		{
			ComponentReloader reloader;
			reloader.entityID = ids.x;
			reloader.componentID = (uint32_t)ids.y;
			reloader.componentPtr = value;
			if (!componentReloaders.has_value())
				componentReloaders = std::vector<ComponentReloader>();
			componentReloaders->push_back(reloader);
		}
	}
#pragma endregion

}

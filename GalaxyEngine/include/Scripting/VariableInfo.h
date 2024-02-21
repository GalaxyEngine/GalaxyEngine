#pragma once
#include "GalaxyAPI.h"
#include <optional>
#include <functional>
namespace GS { struct Property; }
namespace GALAXY
{
	namespace Core { class GameObject; }
	namespace Resource { class Scene; }
	namespace Scripting {
		enum class VariableType
		{
			None,
			Unknown,
			Bool,
			Int,
			Float,
			Double,
			String,
			Vector2f,
			Vector3f,
			Vector4f,
			Quaternion,
			GameObject,
			Component,
		};

		struct GameObjectReloader
		{
			void* gameObjectPtr;
			uint64_t entityID = -1;
		};

		struct ComponentReloader
		{
			void* componentPtr;
			uint64_t entityID = -1;
			uint32_t componentID = -1;
		};

		struct VariableInfo
		{
		public:
			VariableInfo() = default;
			VariableInfo(const GS::Property& variable);

			void Initialize(const GS::Property& variable);
			static void SanitizeType(std::string& typeName);

			std::vector<std::string> args;
			std::string name;
			std::string basetypeName;
			std::string typeName;

			VariableType type = VariableType::None;
			bool isAList = false;
			std::function<void(const std::string& name, void* value)> displayValue = nullptr;

			static void BeginSerializeList(CppSer::Serializer& serializer, const std::string& name, size_t size);
			void BeginSerialize(CppSer::Serializer& serializer, const std::string& name) const;
			static size_t BeginDeserializeList(CppSer::Parser& parser, const std::string& name);
			bool BeginDeserialize(CppSer::Parser& parser, const std::string& name) const;

			virtual void Deserialize(CppSer::Parser& parser, const std::string& name, void* value) {}
			virtual void Serialize(CppSer::Serializer& serializer, const std::string& name, void* value) const {}

			void AfterLoad(Resource::Scene* scene);

			static VariableType TypeNameToType(const std::string& typeName);

			std::optional< std::vector<GameObjectReloader>> gameObjectReloaders;
			std::optional<std::vector<ComponentReloader>> componentReloaders;
		};

		template <typename T>
		struct VariableInfoT : public VariableInfo
		{
		public:
			VariableInfoT(const GS::Property& variable)
			{
				Initialize(variable);
				ManageValue();
			}
		private:
			inline void ManageValue()
			{
				if (isAList)
				{
					this->displayValue = [&](const std::string& name, void* value) {
						std::vector<T>* list = (std::vector<T>*)value;
						if (!list)
							return

							Wrapper::GUI::SetNextItemOpen();
						if (Wrapper::GUI::TreeNode(name.c_str())) {
							for (size_t i = 0; i < list->size(); i++) {
								Wrapper::GUI::PushID(static_cast<int>(i));
								T valueList = (*list)[i];

								DisplayValue(name, &valueList);
								(*list)[i] = valueList;
								Wrapper::GUI::PopID();
							}
							if (Wrapper::GUI::Button("+")) {
								list->push_back({});
							}
							Wrapper::GUI::SameLine();
							if (Wrapper::GUI::Button("-") && !list->empty()) {
								list->pop_back();
							}
							Wrapper::GUI::TreePop();
						}
						};
				}
				else
				{
					auto boundFunction = std::bind(
						&Scripting::VariableInfoT<T>::DisplayValue,
						this,
						std::placeholders::_1,
						std::placeholders::_2
					);

					displayValue = boundFunction;
				}
			}

			void DisplayValue(const std::string& name, void* value);

			void Serialize(CppSer::Serializer& serializer, const std::string& name, void* value) const override
			{
				BeginSerialize(serializer, name);
				if (isAList)
				{
					std::vector<T> vectorValue = *(std::vector<T>*)value;
					VariableInfo::BeginSerializeList(serializer, name, vectorValue.size());
					size_t i = 0;
					if constexpr (std::is_same_v<T, bool>) {
						for (auto v : vectorValue)
						{
							SerializeT(serializer, name + " " + std::to_string(i++), &v);
						}	
					}
					else {
						for (auto& v : vectorValue)
						{
							SerializeT(serializer, name + " " + std::to_string(i++), &v);
						}	
					}
				}
				else
				{
					SerializeT(serializer, name, value);
				}
			}

			void Deserialize(CppSer::Parser& parser, const std::string& name, void* value) override
			{
				if (!BeginDeserialize(parser, name))
					return;
				if (isAList)
				{
					std::vector<T>* vectorValue = (std::vector<T>*)value;
					vectorValue->clear();
					size_t size = VariableInfo::BeginDeserializeList(parser, name);
					if (size == -1)
						return;
					vectorValue->resize(size);
					if constexpr (std::is_same_v<T, bool>) {
						for (size_t i = 0; i < size; i++)
						{
							// Handle the bool case separately
							bool element = (*vectorValue)[i];
							DeserializeT(parser, name + " " + std::to_string(i), &element);
							(*vectorValue)[i] = element; // Assign the deserialized value back to the vector
						}
					} 
					else {
						for (size_t i = 0; i < size; i++)
						{
							// For other types, your existing code can be used
							T& element = (*vectorValue)[i];
							DeserializeT(parser, name + " " + std::to_string(i), &element);
						}
					}
				}
				else
				{
					DeserializeT(parser, name, value);
				}
			}

			void SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const;
			void DeserializeT(CppSer::Parser& parser, const std::string& name, void* value);
		};
	}
}
#include "Scripting/VariableInfo.inl" 

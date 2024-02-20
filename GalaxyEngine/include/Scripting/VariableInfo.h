#pragma once
#include "GalaxyAPI.h"

#include <functional>
namespace GS { struct Property; }
namespace GALAXY
{
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
		};

		struct VariableInfo
		{
		public:
			VariableInfo() = default;
			VariableInfo(const GS::Property& variable);

			void Initialize(const GS::Property& variable);

			std::vector<std::string> args;
			std::string name;
			std::string typeName;

			VariableType type = VariableType::None;
			bool isAList = false;
			std::function<void(const std::string& name, void* value)> displayValue = nullptr;

			static void BeginSerializeList(CppSer::Serializer& serializer, const std::string& name, size_t size);
			static size_t BeginDeserializeList(CppSer::Parser& parser, const std::string& name);

			virtual void Deserialize(CppSer::Parser& parser, const std::string& name, void* value) const {}
			virtual void Serialize(CppSer::Serializer& serializer, const std::string& name, void* value) const {}

			static VariableType TypeNameToType(const std::string& typeName);
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
				if (isAList)
				{
					std::vector<T> vectorValue = *(std::vector<T>*)value;
					VariableInfo::BeginSerializeList(serializer, name, vectorValue.size());
					size_t i = 0;
					for (auto& v : vectorValue)
					{
						SerializeT(serializer, name + " " + std::to_string(i++) , &v);
					}
				}
				else
				{
					SerializeT(serializer, name, value);
				}
			}

			void Deserialize(CppSer::Parser& parser, const std::string& name, void* value) const override
			{
				if (isAList)
				{
					std::vector<T>* vectorValue = (std::vector<T>*)value;
					vectorValue->clear();
					size_t size = VariableInfo::BeginDeserializeList(parser, name);
					for (size_t i = 0; i < size; i++)
					{
						if (vectorValue->size() <= i)
						{
							vectorValue->push_back({});
						}
						DeserializeT(parser, name + " " + std::to_string(i), &vectorValue->operator[](i));
					}
				}
				else
				{
					DeserializeT(parser, name, value);
				}
			}

			void SerializeT(CppSer::Serializer& serializer, const std::string& name, void* value) const;
			void DeserializeT(CppSer::Parser& parser, const std::string& name, void* value) const;
		};
	}
}
#include "Scripting/VariableInfo.inl" 

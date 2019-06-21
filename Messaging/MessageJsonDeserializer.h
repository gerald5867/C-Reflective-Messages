#pragma once
#include "Message.h"
#include "Json.h"
#include "MessageJsonSerializerBase.h"

namespace messaging {
namespace INTERNAL {

class JsonDeserializer final : public JsonSerializerBase {

public:
	JsonDeserializer() = default;
	JsonDeserializer(const JsonDeserializer&) = delete;
	JsonDeserializer& operator =(const JsonDeserializer&) = delete;

	template<typename DerivedType, typename... FieldTypes>
	void Deserialize(BasicMessage<DerivedType, FieldTypes...>& msg, const nlohmann::json& jObj) {
		m_curJson = &jObj;
		msg.ForEachField([this](auto& field, auto Idx) {
			if (IsValidField(Idx)) {
				DeserializeOne(DerivedType::FieldNameStrings[Idx].data(), field);
			}
		});
	}

	template<typename T,
		std::enable_if_t<std::is_base_of<IMessage, RemoveCVREF<T>>::value, bool> Dummy = false, 
	typename FieldStr, std::size_t N>
	void DeserializeOne(const FieldStr& str, std::array<T, N>& msgContainer) {
		auto nestJsonObj = (*m_curJson)[str].get<std::array<nlohmann::json, N>>();
		std::size_t nestObjIdx = 0;
		for (auto& msg : msgContainer) {
			JsonDeserializer ser;
			ser.Deserialize(msg, nestJsonObj[nestObjIdx++]);
		}
	}

	template<typename T,
		std::enable_if_t<std::is_base_of<IMessage, RemoveCVREF<T>>::value, bool> Dummy = false, 
	typename FieldStr>
	void DeserializeOne(const FieldStr& str, std::vector<T>& msgContainer) {
		auto nestJsonObj = (*m_curJson)[str].get<std::vector<nlohmann::json>>();
		msgContainer.resize(nestJsonObj.size());
		std::size_t nestObjIdx = 0;
		for (auto& msg : msgContainer) {
			JsonDeserializer ser;
			ser.Deserialize(msg, nestJsonObj[nestObjIdx++]);
		}
	}


	template<typename DerivedType, typename... FieldTypes, typename FieldStr>
	void DeserializeOne(const FieldStr& str, BasicMessage<DerivedType, FieldTypes...>& msg) {
		JsonDeserializer ser;
		ser.Deserialize(msg, (*m_curJson)[str]);
	}
	

	template<typename T, 
		std::enable_if_t<(!std::is_base_of<IMessage, INTERNAL::RemoveCVREF<T>>::value) && (!IsContainerWithMessages<T>), bool> Dummy = false, typename FieldStr>
	void DeserializeOne(const FieldStr& str, T& field) {
		field = (*m_curJson)[str].get<INTERNAL::RemoveCVREF<T>>();
	}


private:
	const nlohmann::json* m_curJson;

};
}
namespace json_serilization {
	template<typename MessageType>
	inline void Deserialize(MessageType& msg, const std::string& jsonStr, 
		const std::vector<std::size_t>& NonSerializeableFields = std::vector<std::size_t>{}) 
	{
		try {
			nlohmann::json jObj = nlohmann::json::parse(jsonStr);
			INTERNAL::JsonDeserializer ser;
			ser.SetNonSerializeableFields(NonSerializeableFields);
			ser.Deserialize(msg, jObj);
		} catch (const nlohmann::json::parse_error& ex) {
			throw JsonSerilizationException(ex.what());
		}
	}
}
}
#pragma once
#include <cstdio>
#include "Message.h"
#include "Json.h"
#include "MessageJsonSerializerBase.h"
namespace messaging {
namespace INTERNAL {

class JsonSerializer final : public JsonSerializerBase {

public:
	JsonSerializer() = default;
	JsonSerializer(const JsonSerializer&) = delete;
	JsonSerializer& operator =(const JsonSerializer&) = delete;

	template<typename DerivedType, typename... FieldTypes>
	nlohmann::json& Serialize(const BasicMessage<DerivedType, FieldTypes...>& msg) {
		msg.ForEachField([this](const auto& field, auto Idx) {
			if (IsValidField(Idx)) {
				SerializeOne(DerivedType::FieldNameStrings[Idx].data(), field);
			}
		});
		return m_curJson;
	}

	template<typename ContainerType, 
		std::enable_if_t<IsContainerWithMessages<ContainerType>, bool> Dummy = false, typename FieldStr>
	void SerializeOne(const FieldStr& str, const ContainerType& msgContainer) {
		std::vector<nlohmann::json> jsonObjs;
		jsonObjs.reserve(msgContainer.size());
		for (const auto& msg : msgContainer) {
			JsonSerializer ser;
			jsonObjs.emplace_back(std::move(ser.Serialize(msg)));
		}
		m_curJson[str] = jsonObjs;
	}


	template<typename DerivedType, typename... FieldTypes, typename FieldStr>
	void SerializeOne(const FieldStr& str, const BasicMessage<DerivedType, FieldTypes...>& msg) {
		JsonSerializer ser;
		m_curJson[str] = ser.Serialize(msg);
	}
	

	template<typename T, 
		std::enable_if_t<(!std::is_base_of<IMessage, INTERNAL::RemoveCVREF<T>>::value) 
		&& (!IsContainerWithMessages<T>), bool> Dummy = false, typename FieldStr>
	void SerializeOne(const FieldStr& str, const T& field) {
		m_curJson[str] = field;
	}


private:
	nlohmann::json m_curJson;
};
}
namespace json_serilization {
	template<typename MessageType>
	inline std::string Serialize(const MessageType& msg, 
		const std::vector<std::size_t>& NonSerializeableFields = std::vector<std::size_t>{}) {
		INTERNAL::JsonSerializer ser;
		try {
			ser.SetNonSerializeableFields(NonSerializeableFields);
			const nlohmann::json& js = ser.Serialize(msg);
			return js.dump(2);
		} catch (const nlohmann::json::parse_error& ex) {
			throw JsonSerilizationException(ex.what());
		}
	}
}
}
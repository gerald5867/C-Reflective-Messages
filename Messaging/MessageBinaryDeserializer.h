#pragma once
#include <vector>
#include <type_traits>
#include <array>
#include <string>
#include "BasicMessage.h"
#include "MessagingTupleUtils.h"

namespace messaging {

namespace INTERNAL {
	template<typename T>
	using EnableBoolIfIsTrivial = std::enable_if_t<std::is_trivially_copyable<INTERNAL::RemoveCVREF<T>>::value, bool>;

	template<typename T>
	using EnableBoolIfNotIsTrivial = std::enable_if_t<(!std::is_trivially_copyable<INTERNAL::RemoveCVREF<T>>::value), bool>;

class BinaryDeserializer final {
public:
	BinaryDeserializer() = default;
	BinaryDeserializer(const BinaryDeserializer&) = delete;
	BinaryDeserializer& operator = (const BinaryDeserializer&) = delete;

	template<typename DerivedType, typename... MessageTypes>
	std::size_t Deserialize(BasicMessage<DerivedType, MessageTypes...>& message, const Byte* pSource, const std::int32_t len) {
		m_len = len;
		auto* pStart = pSource;
		m_curPtr = pSource;
		m_endPtr = m_curPtr + m_len;
		message.ForEachArrayFieldDo([this](auto& array, const std::size_t Index) {
			static_assert(INTERNAL::IsStdArray<INTERNAL::RemoveCVREF<decltype(array)>>::Value, "Weired that should be a std::array!!!");
			DeserializeOne(array);
		});
		return m_curPtr - pStart;
	}

private:
	const Byte* m_curPtr = nullptr;
	const Byte* m_endPtr = nullptr;
	std::int32_t m_len = -1;

	void DoSizeCheck(const std::int32_t len) const {
		if (m_len != -1 && m_curPtr + len > m_endPtr) {
			throw std::runtime_error("bytes from client were lower then expected!! got : "+std::to_string(m_len));
		}
	}

	template<typename T, INTERNAL::EnableBoolIfIsTrivial<T> Dummy = false>
	void DeserializeOne(T& field) {
		constexpr std::size_t FieldSize = sizeof(INTERNAL::RemoveCVREF<T>);
		DoSizeCheck(FieldSize);
		std::memcpy(reinterpret_cast<void*>(&field), m_curPtr, FieldSize);
		m_curPtr += FieldSize;
	}


	template<typename DerivedType, typename...FieldTypes>
	void DeserializeOne(BasicMessage<DerivedType, FieldTypes...>& childMsg) {
		m_curPtr += binary_serilization::Deserialize(childMsg, m_curPtr, m_endPtr - m_curPtr);
	}


	void DeserializeOne(std::string& str) {
		const std::size_t StrSize = DeserializeBufferLen();
		DoSizeCheck(StrSize);
		str.assign(reinterpret_cast<const std::string::value_type*>(m_curPtr), StrSize);
		m_curPtr += (str.size() * sizeof(std::string::value_type));
	}


	template<typename T, INTERNAL::EnableBoolIfIsTrivial<T> Dummy = false>
	void DeserializeOne(std::vector<T>& field) {
		using VecValT = typename std::vector<T>::value_type;
		auto bufLen = DeserializeBufferLen();
		field.resize(bufLen);
		std::memcpy(field.data(), m_curPtr, field.size() * sizeof(VecValT));
		m_curPtr += (field.size() * sizeof(VecValT));
	}


	void DeserializeOne(std::vector<bool>& field) {
		auto bufLen = DeserializeBufferLen();
		DoSizeCheck(bufLen);
		field.resize(bufLen);
		for (auto&& val : field) {
			val = *reinterpret_cast<const bool*>(m_curPtr);
			m_curPtr += sizeof(bool);
		}
	}


	std::size_t DeserializeBufferLen() {
		auto ret = *reinterpret_cast<const INTERNAL::SerializedSizeDataType*>(m_curPtr);
		DoSizeCheck(sizeof(INTERNAL::SerializedSizeDataType));
		m_curPtr += sizeof(INTERNAL::SerializedSizeDataType);
		if (ret >= (std::numeric_limits<std::uint16_t>::max)()) {
			throw std::runtime_error{"buffer len was higher than uin16t max!!!!"};
		}
		return ret;
	}


	template<typename T, INTERNAL::EnableBoolIfNotIsTrivial<T> Dummy = false>
	void DeserializeOne(std::vector<T>& field) {
		field.resize(DeserializeBufferLen());
		for (auto& entry : field) {
			DeserializeOne(entry);
		}
	}


	template<typename T, std::size_t N, INTERNAL::EnableBoolIfNotIsTrivial<T> Dummy = false>
	void DeserializeOne(std::array<T, N>& field) {
		for (auto& entry : field) {
			DeserializeOne(entry);
		}
	}
};
}//namespace INTERNAL
namespace binary_serilization {

	template<typename DerivedType, typename... MessageTypes>
	inline std::size_t Deserialize(BasicMessage<DerivedType, MessageTypes...>& message, const Byte* pSource, const std::int32_t len = -1) {
		INTERNAL::BinaryDeserializer s;
		return s.Deserialize(message, pSource, len);
	}

	template<typename DerivedType, typename... MessageTypes>
	inline std::size_t Deserialize(BasicMessage<DerivedType, MessageTypes...>& message, const std::vector<Byte>& pSource) {
		INTERNAL::BinaryDeserializer s;
		return s.Deserialize(message, pSource.data(), pSource.size());
	}

}//namespace binary_serilization
}//namespace messaging
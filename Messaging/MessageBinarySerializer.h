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

class BinarySerializer final {
public:
	BinarySerializer() = default;

	BinarySerializer(const BinarySerializer&) = delete;
	BinarySerializer& operator =(const BinarySerializer&) = delete;

	template<typename DerivedType, typename... MessageTypes>
	inline std::size_t Serialize(const BasicMessage<DerivedType, MessageTypes...>& message, Byte* pDestination) {
		auto* start = pDestination;
		m_pDest = pDestination;
		message.ForEachArrayFieldDo([this](const auto& array, const std::size_t Index) {
			static_assert(INTERNAL::IsStdArray<INTERNAL::RemoveCVREF<decltype(array)>>::Value, "Weired that should be a std::array!!!");
			SerializeOne(array);
		});
		return m_pDest - start;
	}


	template<typename DerivedType, typename... MessageTypes>
	inline std::vector<Byte> Serialize(const BasicMessage<DerivedType, MessageTypes...>& message) {
		std::vector<Byte> result;
		result.resize(message.GetMessageSize());
		if (result.size() < Serialize(message, result.data())) {
			throw std::runtime_error("FATAL ERROR !!! ACCESS VIOLATION!!!");
		}
		return result;
	}

private:
	Byte* m_pDest = nullptr;


	template<typename T, INTERNAL::EnableBoolIfIsTrivial<T> Dummy = false>
	void SerializeOne(const T& field) {
		using Type = INTERNAL::RemoveCVREF<T>;
		std::memcpy(m_pDest, reinterpret_cast<const void*>(&field), sizeof(Type));
		m_pDest += sizeof(Type);
	}


	template<typename DerivedType, typename...FieldTypes>
	void SerializeOne(const BasicMessage<DerivedType, FieldTypes...>& childMsg) {
		m_pDest += binary_serilization::Serialize(childMsg, m_pDest);
	}


	void SerializeOne(const std::string& str) {
		SerializeBufferCount(str.size());
		const std::size_t strSize = str.size() * sizeof(std::string::value_type);
		std::memcpy(m_pDest, str.data(), strSize);
		m_pDest += strSize;
	}


	template<typename T, INTERNAL::EnableBoolIfIsTrivial<T> Dummy = false>
	void SerializeOne(const std::vector<T>& field) {
		SerializeBufferCount(field.size());
		const std::size_t VecSize = field.size() * sizeof(typename std::vector<T>::value_type);
		std::memcpy(m_pDest, field.data(), VecSize);
		m_pDest += VecSize;
	}


	void SerializeOne(const std::vector<bool>& field) {
		SerializeBufferCount(field.size());
		for (const auto bmem : field) {
			std::memcpy(m_pDest, &bmem, sizeof(bool));
			m_pDest += sizeof(bool);
		}
	}


	void SerializeBufferCount(const std::size_t BufferLen) {
		const auto bufLen = static_cast<INTERNAL::SerializedSizeDataType>(BufferLen);
		std::memcpy(m_pDest, &bufLen, sizeof(INTERNAL::SerializedSizeDataType));
		m_pDest += sizeof(INTERNAL::SerializedSizeDataType);
	}


	template<typename T, INTERNAL::EnableBoolIfNotIsTrivial<T> Dummy = false>
	void SerializeOne(const std::vector<T>& field) {
		SerializeBufferCount(field.size());
		for (const auto& entry : field) {
			SerializeOne(entry);
		}
	}


	template<typename T, std::size_t N, INTERNAL::EnableBoolIfNotIsTrivial<T> Dummy = false>
	void SerializeOne(const std::array<T, N>& field) {
		for (const auto& entry : field) {
			SerializeOne(entry);
		}
	}
};
}//namespace INTERNAL

namespace binary_serilization {
	template<typename DerivedType, typename... MessageTypes>
	inline std::size_t Serialize(const BasicMessage<DerivedType, MessageTypes...>& message, Byte* pDestination) {
		INTERNAL::BinarySerializer s;
		return s.Serialize(message, pDestination);
	}

	template<typename DerivedType, typename... MessageTypes>
	inline std::vector<Byte> Serialize(const BasicMessage<DerivedType, MessageTypes...>& message) {
		INTERNAL::BinarySerializer s;
		return s.Serialize(message);
	}
}//namespace binary_serilization
}//namespace messaging
#pragma once
#include <array>
#include <vector>
#include <type_traits>
#include "../utils/ConstexprStringUtils.h"
#include "../utils/ConstexprStringView.h"

namespace messaging {
	template<typename, typename...> class BasicMessage;
	class IMessage;

	template<typename T, std::size_t N> using StaticMessageArray = std::array<T, N>;
	template<typename T> using DnymaicMessageArray = std::vector<T>;
	using DynamicMessageString = std::string;

namespace INTERNAL {
	class EnumTag;

	using SerializedSizeDataType = std::uint32_t;

	template<typename T>
	using RemoveCVREF = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename T>
	using AddConstLVReference = std::add_lvalue_reference_t<std::add_const_t<T>>;

	template<typename T, typename = void>
	struct EnumConverter {
		using Type = INTERNAL::RemoveCVREF<T>;
	};

	template<typename T>
	struct EnumConverter<T, std::enable_if_t<std::is_enum<INTERNAL::RemoveCVREF<T>>::value>> {
		using Type = std::underlying_type_t<INTERNAL::RemoveCVREF<T>>;
	};

	template<typename> 
	struct IsStdArray {
		static constexpr bool Value = false;
	};

	template<typename T, std::size_t N>
	struct IsStdArray<std::array<T, N>> {
		static constexpr bool Value = true;
	};

	template<typename>
	struct IsStdVector {
		static constexpr bool Value = false;
	};

	template<typename T>
	struct IsStdVector<std::vector<T>> {
		static constexpr bool Value = true;
	};

	template<typename T, typename DecayedT = INTERNAL::RemoveCVREF<T>> 
	constexpr bool IsDynamicOrStaticArray = IsStdVector<DecayedT>::Value || IsStdArray<DecayedT>::Value;

	template<typename T, typename DecayedT = INTERNAL::RemoveCVREF<T>>
	constexpr bool IsDynamicString = std::is_same<DecayedT, std::string>::value;

	template<typename>
	struct IsStaticTuple;

	template<>
	struct IsStaticTuple<std::tuple<>> {
		static constexpr bool Value = true;
	};


	template<typename T> constexpr bool IsStaticNestedMsg(std::false_type) { return false; }
	template<typename T> constexpr bool IsStaticNestedMsg(std::true_type) { return IsStaticTuple<typename T::TupleFieldTypes>::Value; }


	template<typename FirstFieldType, typename... RestFieldTypes>
	struct IsStaticTuple<std::tuple<FirstFieldType, RestFieldTypes...>> {
		using FirstFieldDecayed = RemoveCVREF<FirstFieldType>;
		static constexpr bool Value = (std::is_trivially_copyable<FirstFieldDecayed>::value ||
			IsStaticNestedMsg<FirstFieldDecayed>(std::is_base_of<IMessage, FirstFieldDecayed>{}))
			&& IsStaticTuple<std::tuple<RestFieldTypes...>>::Value;
	};


		template<typename, typename> struct CompareTupleElements;

		template<> struct CompareTupleElements <std::tuple<>, std::tuple<>> {
			using Type = void;
		};

		template<typename FirstLeftType, typename... LeftTupleElementTypes,
			typename FirstRightType, typename... RightTupleElementTypes>
			struct CompareTupleElements <std::tuple<FirstLeftType, LeftTupleElementTypes...>, std::tuple<FirstRightType, RightTupleElementTypes...>> {
			static_assert(sizeof...(LeftTupleElementTypes) == sizeof...(RightTupleElementTypes),
				"The Argument Count does not Match the member count of the Message!");
			using LeftDecayed = typename INTERNAL::EnumConverter<INTERNAL::RemoveCVREF<FirstLeftType>>::Type;
			using RightDecayed = typename INTERNAL::EnumConverter<INTERNAL::RemoveCVREF<FirstRightType>>::Type;
			static_assert(std::is_same<LeftDecayed, RightDecayed>::value || std::is_convertible<LeftDecayed, RightDecayed>::value,
				"One or more arguments dont match the message arguments!!!");
			using Type = typename CompareTupleElements<std::tuple<LeftTupleElementTypes...>, std::tuple<RightTupleElementTypes...>>::Type;
		};

		template<typename T, typename = std::enable_if_t<std::is_arithmetic<RemoveCVREF<T>>::value>>
		struct SizeOfMessageField {
			static constexpr std::size_t Size = sizeof(RemoveCVREF<T>);
		};

		template<typename T, std::size_t N, typename SFINAEDummy>
		struct SizeOfMessageField<std::array<T, N>, SFINAEDummy > {
			static constexpr std::size_t Size = sizeof(RemoveCVREF<T>) * N;
		};

		template<typename MessageType>
		struct SizeOfMessageField<MessageType, std::enable_if_t<std::is_base_of<IMessage, RemoveCVREF<MessageType>>::value>> {
			static constexpr std::size_t Size = MessageType::GetStaticMessageSize();
		};


		template<typename T, bool SecondCond = true>
		using EnableBoolIfTrivial= std::enable_if_t<std::is_trivially_copyable<RemoveCVREF<T>>::value
			&& SecondCond, bool>;

		template<typename T, bool SecondCond = true>
		using EnableBoolIfNotTrivial = std::enable_if_t<(!std::is_trivially_copyable<RemoveCVREF<T>>::value)
			&& SecondCond, bool>;


		template<typename T, EnableBoolIfTrivial<T> Dummy = false>
		constexpr std::size_t DynamicSizeOfMessageField(const T& val) noexcept {
			return sizeof(RemoveCVREF<T>);
		}

		template<typename T, EnableBoolIfNotTrivial<T, std::is_base_of<IMessage, RemoveCVREF<T>>::value> Dummy = false>
		constexpr std::size_t DynamicSizeOfMessageField(const T& val) noexcept {
			return val.GetMessageSize();
		}
		
		inline std::size_t DynamicSizeOfMessageField(const std::string& val) noexcept {
			return (val.size() * sizeof(std::string::value_type)) + sizeof(SerializedSizeDataType);
		}

		template<typename T, EnableBoolIfNotTrivial<T> Dummy = false>
		constexpr std::size_t DynamicSizeOfMessageField(const std::vector<T>& val) noexcept {
			std::size_t res = sizeof(SerializedSizeDataType);
			for (const auto& entry : val) {
				res += DynamicSizeOfMessageField(entry);
			}
			return res;
		}

		template<typename T, EnableBoolIfTrivial<T> Dummy = false>
		constexpr std::size_t DynamicSizeOfMessageField(const std::vector<T>& val) noexcept {
			return (val.size() * sizeof(typename std::vector<T>::value_type)) + sizeof(SerializedSizeDataType);
		}

		template<typename T, std::size_t N, EnableBoolIfNotTrivial<T> Dummy = false>
		constexpr std::size_t DynamicSizeOfMessageField(const std::array<T, N>& val) noexcept {
			std::size_t res = 0;
			for (const auto& entry : val) {
				res += DynamicSizeOfMessageField(entry);
			}
			return res;
		}

		template<typename T> constexpr bool IsMessageContainer(std::false_type) { return false; }
		template<typename T> constexpr bool IsMessageContainer(std::true_type) {
			return std::is_base_of<IMessage, typename RemoveCVREF<T>::value_type>::value;
		}

		template<typename ContainerType>
		constexpr bool IsStdVectorOfMessages = IsMessageContainer<ContainerType>(
			std::bool_constant<IsStdVector<RemoveCVREF<ContainerType>>::Value>{});

		template<typename ContainerType>
		constexpr bool IsStdArrayOfMessages = IsMessageContainer<ContainerType>(
			std::bool_constant<IsStdArray<RemoveCVREF<ContainerType>>::Value>{});

		template<typename ContainerType>
		constexpr bool IsContainerWithMessages = IsStdVectorOfMessages<ContainerType> || IsStdArrayOfMessages<ContainerType>;
}//namespace INTERNAL

	template<typename T>
	class ArrayView final {
	public:
		template<std::size_t N>
		explicit ArrayView(const std::array<T, N>& field) : m_ptr(field.data()), m_cLen(field.size()) {}
		ArrayView(const T* const ptr, const std::size_t cLen) : m_ptr(ptr), m_cLen(cLen) {}
		inline const T* cbegin() const noexcept { return m_ptr; }
		inline const T* begin() noexcept { return m_ptr; }
		inline const T* cend() const noexcept { return m_ptr + m_cLen; }
		inline const T* end() noexcept { return m_ptr + m_cLen; }
		inline std::size_t Count() const noexcept { return m_cLen; }
		inline const T* Data() { return m_ptr; }
		inline const T& operator[](const std::size_t Index) const { return m_ptr[Index]; }
	private:
		const T* const m_ptr = nullptr;
		const std::size_t m_cLen = 0;
	};

	enum class Byte : std::uint8_t {};



	template<typename T>
	constexpr std::size_t ArraySize(const T& arr) { return arr.size(); }

	template<typename T, std::size_t N> 
	constexpr std::size_t ArraySize(const T(&arr)[N]) { return N; }

	template<typename LeftArr, typename RightArr>
	constexpr bool ArrayCompare(const LeftArr& left, const RightArr& right) noexcept {
		if (ArraySize(left) != ArraySize(right)) {
			return false;
		}
		for (std::size_t i = 0, end = ArraySize(left); i < end; ++i) {
			if (left[i] != right[i]) {
				return false;
			}
		}
		return true;
	}


	//Note prior to C++17 std::arrays non const functions were not constexpr
	template<typename T, std::size_t LEN>
	struct ConstexprArray {
		constexpr ConstexprArray() = default;
		constexpr ConstexprArray(const T(&otherArray)[LEN]) {
			for (auto i = 0; i < LEN; ++i) {
				m_data[i] = otherArray[i];
			}
		}

		template<typename... ValTypes>
		constexpr ConstexprArray(ValTypes&&... values) : m_data{ values... } {}

		constexpr T& operator[](const std::size_t Idx) {
			if (Idx >= LEN) {
				throw std::runtime_error("Index out of range!!!");
			}
			return m_data[Idx];
		}

		constexpr const T& operator[](const std::size_t Idx) const {
			return const_cast<ConstexprArray*>(this)->operator[](Idx);
		}

		constexpr bool Contains(const T& toSearch) const noexcept {
			return IndexOf(toSearch) != -1;
		}

		 
		constexpr std::int64_t IndexOf(const T& toSearch) const noexcept {
			for (std::size_t i = 0, end = size(); i < end; ++i) {
				if (m_data[i] == toSearch) {
					return i;
				}
			}
			return -1;
		}

		template<typename PredicateType, std::enable_if_t<(!std::is_same<INTERNAL::RemoveCVREF<PredicateType>, T>::value && 
			!std::is_enum<INTERNAL::RemoveCVREF<PredicateType>>::value), bool> Dummy = false>
		constexpr std::int64_t IndexOf(PredicateType&& pred) const noexcept {
			for (std::size_t i = 0, end = size(); i < end; ++i) {
				if (pred(m_data[i])) {
					return i;
				}
			}
			return -1;
		}


		inline constexpr std::size_t size() const noexcept { return LEN; }
		inline constexpr const T* begin() const noexcept { return m_data; }
		inline constexpr const T* cbegin() const noexcept { return m_data; }
		inline constexpr const T* end() const noexcept { return m_data + LEN; }
		inline constexpr const T* cend() const noexcept { return m_data + LEN; }
		inline constexpr const T* data() const noexcept { return m_data; }
		inline constexpr T* data() noexcept { return m_data; }
		T m_data[LEN] = {};
	};

namespace INTERNAL {

	template<std::size_t N>
	constexpr std::size_t CountEnumValues(const char(&enumstring)[N]) noexcept {
		std::size_t count = 0;
		for (auto ch : enumstring) {
			if (ch == ',') {
				++count;
			}
		}
		return count + 1;
	}

	template<std::size_t N>
	constexpr std::size_t GetLongestEnumValue(const char(&enumstring)[N]) noexcept {
		std::int32_t longestLen = 0;
		std::int32_t lastFoundIndex = 0;
		for (std::int32_t i = 0; i < (N - 1); ++i) {
			if (enumstring[i] == ',') {
				if (i - lastFoundIndex > longestLen) {
					longestLen = i - lastFoundIndex;
				}
				lastFoundIndex = i;
			}
		}
		if (static_cast<std::int32_t>(((N - 1) - lastFoundIndex)) > longestLen) {
			longestLen = (N - 1) - lastFoundIndex;
		}
		return longestLen;
	}

	template<std::size_t N>
	constexpr bool VerifyUniqueMessageKeys(const std::array<utils::ConstexprStringView, N>& keys) noexcept {
		for (std::size_t i = 0; i < N; ++i) {
			for (std::size_t j = 0; j < N; ++j) {
				if (i != j && keys[i] == keys[j]) {
					return false;
				}
			}
		}
		return true;
	}

	template<std::size_t N>
	constexpr decltype(auto) CreateMessageKeyLens(const std::array<utils::ConstexprStringView, N>& views) noexcept {
		ConstexprArray<std::size_t, N> lenResult = {};
		std::size_t i = 0;
		for (std::size_t i = 0; i < N; ++i) {
			lenResult[i] = views[i].size();
		}
		return lenResult;
	}

	//todo make a constexpr count

	template<std::size_t EnumCount, std::size_t LongestEnumValue, std::size_t N>
	constexpr decltype(auto) CreateEnumStrings(const char(&enumstring)[N]) noexcept {
		ConstexprArray<ConstexprArray<char, LongestEnumValue>, EnumCount> parsedEnums = {};
		std::int32_t currentIndex = 0;
		std::int32_t lastDelimFound = 0;
		for (std::int32_t chIter = 0; chIter < (N - 1); ++chIter) {
			if (enumstring[chIter] == ',' && chIter + 1 < (N - 1) && enumstring[chIter + 1] == ' ') {
				for (std::int32_t i = lastDelimFound; i < chIter; ++i) {
					parsedEnums[currentIndex][i - lastDelimFound] = enumstring[i];
				}
				++currentIndex;
				++chIter;
				lastDelimFound = chIter + 1;
			}
		}
		for (std::int32_t i = lastDelimFound; i < (N - 1); ++i) {
			parsedEnums[currentIndex][i - lastDelimFound] = enumstring[i];
		}
		return parsedEnums;
	}

} // namespace INTERNAL
}
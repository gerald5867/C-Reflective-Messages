#pragma once
#include <type_traits>
#include <algorithm>
#include <array>
#include <limits>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "../Messaging/MessageHelpers.h"
#include "../utils/ConstexprStringView.h"
#include "../utils/ConstexprStringUtils.h"


namespace Enum_INTERNAL {
	template<typename EnumType>
	constexpr auto FromString(const char* str, std::size_t len = 0) noexcept {
		if (str == nullptr || *str == 0) {
			return EnumType{ EnumType::None };
		}
		if(len == 0) { 
			len = utils::StrLen(str); 
		} 
		std::int64_t Idx = -1;
		const utils::ConstexprStringView strView{str, len};
		const auto& names = EnumType::GetNames();
		for (std::size_t i = 0, end = names.size(); i < end; ++i) {
			if (names[i] == strView) {
				Idx = i;
				break;
			}
		}
		if(Idx == -1 ) { 
			return EnumType{EnumType::None};
		} 
		return EnumType{EnumType::GetValues()[static_cast<std::size_t>(Idx)]};
	}

}

#define STRINGIZE_TOCONSTEXPRVIEW(r, argument, i, e) ::utils::ConstexprStringView{BOOST_PP_STRINGIZE(e)},

#define DECLENUMEX(enumName, enumType, ...) \
	class enumName final { \
	public: \
		static constexpr std::array<utils::ConstexprStringView, BOOST_PP_ADD(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1)> EnumStrings = {\
			BOOST_PP_EXPAND(BOOST_PP_SEQ_FOR_EACH_I(STRINGIZE_TOCONSTEXPRVIEW, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))) \
			::utils::ConstexprStringView{"None"} \
		}; \
		enum enumName##_value : enumType {__VA_ARGS__, None = (std::numeric_limits<enumType>::max)()}; \
		using IntegralType = enumType; \
		constexpr enumName(enumName##_value val) : m_value(val){} \
		constexpr enumName() = default; \
		constexpr explicit operator IntegralType() const noexcept { return static_cast<IntegralType>(m_value); } \
		constexpr operator enumName##_value () const noexcept { return m_value; } \
		static constexpr auto MinValue() noexcept { return (std::min)({__VA_ARGS__}); } \
		static constexpr auto MaxValue() noexcept { return (std::max)({__VA_ARGS__}); } \
		static constexpr std::size_t GetValueCount() noexcept { return EnumStrings.size(); } \
		static constexpr decltype(auto) GetValues() noexcept { \
			static constexpr messaging::ConstexprArray<enumName, GetValueCount()> AllValues = { __VA_ARGS__, None }; \
			return AllValues; \
		} \
		static constexpr decltype(auto) GetNames() noexcept { return EnumStrings; } \
		static constexpr auto FromString(const char* str, std::size_t Len = 0) { return Enum_INTERNAL::FromString<enumName>(str, Len); } \
		static constexpr auto FromString(const std::string& str) { return Enum_INTERNAL::FromString<enumName>(str.c_str(), str.size()); } \
		static constexpr auto FromIntegral(const enumType val) noexcept { return enumName{static_cast<enumName##_value>(val)}; } \
		static constexpr auto ToString(enumName val) { \
			const std::int64_t Idx = GetValues().IndexOf(val); \
			if(Idx == -1) { return EnumStrings[EnumStrings.size() - 1]; } \
			return EnumStrings[static_cast<std::size_t>(Idx)]; \
		} \
		constexpr auto ToString() const noexcept { return ToString(m_value); } \
		constexpr auto ToIntegral() const noexcept { return static_cast<IntegralType>(m_value); } \
		static constexpr decltype(auto) GetEnumName() noexcept { \
			static constexpr utils::ConstexprStringView str{BOOST_PP_STRINGIZE(enumName)}; \
			return str; \
		} \
	private: \
		enumName##_value m_value = None; \
	};

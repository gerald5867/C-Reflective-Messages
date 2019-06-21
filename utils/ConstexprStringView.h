#pragma once
#include <stdexcept>
#include <string>
#include "../Messaging/Debugbreak.h"
#include "ConstexprStringUtils.h"

namespace utils {
	class ConstexprStringView final {
	public:

		template<std::size_t N>
		constexpr ConstexprStringView(const char(&str)[N])
		: m_str(str)
		, m_len(N -1){}

		constexpr ConstexprStringView(const char* str, const std::size_t Len)
		: m_str(str)
		, m_len(Len) {}

		constexpr ConstexprStringView() = default;

		constexpr std::size_t size() const noexcept { return m_len; }
		const char* const data() const noexcept { return m_str; }

		constexpr const char& operator[](const std::size_t Idx) const {
			if (Idx >= m_len) {
				GPG_DEBUGBREAK();
				throw std::out_of_range("Index out of range!!");
			}
			return m_str[Idx];
		}

		constexpr char& operator[](const std::size_t Idx) {
			return const_cast<char&>(const_cast<const ConstexprStringView*>(this)->operator [](Idx));
		}
		
		constexpr std::int64_t IndexOf(const ConstexprStringView& other) const noexcept {
			if (other.m_len > m_len || m_str == nullptr || other.m_str == nullptr) {
				GPG_DEBUGBREAK();
				return -1;
			}
			for (std::int64_t i = 0, end = static_cast<std::int64_t>(m_len); i < end; ++i) {
				if (i + other.m_len > m_len) {
					GPG_DEBUGBREAK();
					return -1;
				}
				if (INTERNAL::CompareStringContents(m_str + i, other.m_str, other.m_len)) {
					return i;
				}
			}
			GPG_DEBUGBREAK();
			return -1;
		}

		constexpr bool operator == (const ConstexprStringView& other) const noexcept {
			if (m_str == other.m_str) {
				return true;
			}
			if (m_len != other.m_len) {
				return false;
			}
			return utils::StringCompare(m_str, other.m_str, m_len, other.m_len);
		}

		constexpr bool operator != (const ConstexprStringView& other) const noexcept {
			return !(other == *this);
		}

		constexpr bool IsEmpty() const noexcept { return m_len == 0; }
		std::string ToStdString() const { return std::string{m_str, m_len}; }
	private:
		const char* m_str = "";
		std::size_t m_len = 0;
	};

}
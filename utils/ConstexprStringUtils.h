#pragma once
#include <cstdint>
#include <cstddef>
namespace utils {
namespace INTERNAL {
	constexpr bool CompareStringContents(const char* left, const char* right, const std::size_t Len) {
		for (std::size_t i = 0; i < Len; ++i) {
			if (left[i] != right[i]) {
				return false;
			}
		}
		return true;
	}
}

	constexpr std::size_t StrCopy(char* dest, const char* source, const std::size_t Len) {
		for (std::size_t i = 0; i < Len; ++i) {
			dest[i] = source[i];
		}
		return Len;
	}


	constexpr inline std::size_t StrLen(const char* str, std::int64_t maxVal = -1) noexcept {
		if (str == nullptr || *str == 0) {
			return 0;
		}
		std::size_t len = 0;
		while (str[len] != 0) { 
			++len;
			if (maxVal != -1 && len >= static_cast<std::size_t>(maxVal)) {
				return 0;
			}
		}
		return len;
	}


	template<std::size_t LeftN, std::size_t RightN>
	constexpr inline bool StringCompare(const char(&leftStr)[LeftN], const char(&rightStr)[RightN]) {
		return (LeftN == RightN) && StringCompare(static_cast<const char*>(leftStr), static_cast<const char*>(rightStr));
	}


	constexpr inline bool StringCompare(const char* left, const char* right, std::size_t leftLen = 0, std::size_t rightLen = 0) {
		leftLen = (leftLen == 0) ? StrLen(left) : leftLen;
		rightLen = (rightLen == 0) ? StrLen(right) : rightLen;
		if (leftLen != rightLen) {
			return false;
		}
		if (left == nullptr || right == nullptr) {
			return false;
		}
		return INTERNAL::CompareStringContents(left, right, leftLen);
	}
}
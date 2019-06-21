#pragma once
#include <array>
#include <algorithm>
#include <initializer_list>
#include "MessageHelpers.h"

namespace messaging {
namespace INTERNAL {
		template<std::size_t...> struct IndiceContainer;

		//we need this due to a bug in C++14 mode in VS2017
		template<typename T, std::size_t N>
		constexpr auto Max(const T(&arr)[N]) {
			T res = 0;
			for (auto it = std::cbegin(arr), end = std::cend(arr); it != end; ++it) {
				res = std::max(res, *it);
			}
			return res;
		}

		template<std::size_t... Offsets>
		constexpr decltype(auto) CreateIndiceContainerZeroOffset(IndiceContainer<Offsets...> dummyContainer) noexcept {
			(void)dummyContainer;
			static constexpr std::size_t OffsetArr[] = { Offsets... };
			static constexpr auto HighestOffset = Max(OffsetArr) + 1;
			ConstexprArray<std::size_t, HighestOffset> result = {};
			std::size_t i = 0;
			(void)std::initializer_list<std::size_t>{(result[Offsets] = i++)...};
			return result;
		}

		template<std::size_t...INDICES>
		struct IndiceContainer {
			static constexpr std::size_t Len = sizeof...(INDICES);
			static constexpr ConstexprArray<std::size_t, Len> Indices = { INDICES... };
			constexpr IndiceContainer() = default;
		};

		template<>
		struct IndiceContainer<> {
			static constexpr std::size_t Len = 0;
			static constexpr ConstexprArray<std::size_t, 1> Indices = {};
			constexpr IndiceContainer() = default;
		};

		template<std::size_t, typename> struct AddIndexToIndiceContainer;

		template<std::size_t NewIndex, std::size_t... OldIndices>
		struct AddIndexToIndiceContainer<NewIndex, IndiceContainer<OldIndices...>> {
			using Type = IndiceContainer<OldIndices..., NewIndex>;
		};

		template<typename, typename, std::size_t CurrentIndex = 0, typename CurrentIndices = IndiceContainer<>>
		struct CreateIndicesByTupleType;

		template<typename ToFindTupleType, std::size_t CurrentIndex, typename CurrentIndiceContainer>
		struct CreateIndicesByTupleType < ToFindTupleType, std::tuple<>, CurrentIndex, CurrentIndiceContainer > {
			using Type = CurrentIndiceContainer;
		};

		template<typename ToFindTupleType, typename CurrentIndiceContainer, std::size_t CurrentIndex, typename FirstTupleType, typename... RestTupleTypes>
		struct CreateIndicesByTupleType < ToFindTupleType, std::tuple<FirstTupleType, RestTupleTypes...>, CurrentIndex, CurrentIndiceContainer > {

			static constexpr bool IsCorrectType =
				std::is_same<RemoveCVREF<FirstTupleType>, RemoveCVREF<ToFindTupleType>>::value;
			using NewIndiceContainer = std::conditional_t <
				IsCorrectType,
				typename AddIndexToIndiceContainer<CurrentIndex, CurrentIndiceContainer>::Type,
				CurrentIndiceContainer
			>;
			using Type = typename CreateIndicesByTupleType<
				ToFindTupleType, std::tuple<RestTupleTypes...>, CurrentIndex + 1, NewIndiceContainer
			>::Type;
		};
}
}
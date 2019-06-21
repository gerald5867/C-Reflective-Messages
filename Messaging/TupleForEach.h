#pragma once
#include <initializer_list>
#include <type_traits>
#include <tuple>
namespace messaging {
namespace INTERNAL {
	template<typename TupleTpe, typename Predicate, std::size_t... Indices>
	constexpr void TupleForEachImpl(TupleType&& tpl, Predicate&& pred, std::index_sequence<Indices>...) {
		(void)std::initializer_list<int>{(pred(std::get<Indices>(tpl)), 0)...};
	}
}
	
	template<typename TupleType, typename Predicate>
	constexpr void TupleForEachDo(TupleType&& tpl, Predicate&& pred) {
		INTERNAL::TupleForEachImpl(
			std::forward<TupleType>(tpl), 
			std::forward<Predicate>(pred),
			std::make_index_sequence<std::tuple_size<std::remove_reference_t<TupleType>>::value>()
		);
	}
}
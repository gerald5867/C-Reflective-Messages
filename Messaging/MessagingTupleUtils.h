#pragma once
#include <tuple>
#include <type_traits>


namespace messaging {
namespace INTERNAL {
	template<typename, typename> struct TupleHasType { using type = void; };
	template <typename T> struct TupleHasType<T, std::tuple<>> : std::false_type {};

	template <typename T, typename U, typename... RestArgTypes>
	struct TupleHasType<T, std::tuple<U, RestArgTypes...>> : TupleHasType<T, std::tuple<RestArgTypes...>> {};

	template <typename T, typename... RestArgTypes>
	struct TupleHasType<T, std::tuple<T, RestArgTypes...>> : std::true_type {};


	template<typename NewTupleMember, template<typename...> class VariadicTemplate,
		typename...CurrentTupleMember>
		auto AddMemberToVariadicTemplate(const VariadicTemplate<CurrentTupleMember...>& dummy) {
		(void)dummy;
		return VariadicTemplate<CurrentTupleMember..., NewTupleMember>{};
	}

	struct NoType {};

	template<std::size_t, typename> struct TupleTypeAt;

	template<std::size_t Idx>
	struct TupleTypeAt<Idx, std::tuple<>> {
		using Type = NoType;
	};

	template<std::size_t Idx, typename... TupleTypes>
	struct TupleTypeAt<Idx, std::tuple<TupleTypes...>> {
		using Type = std::tuple_element_t<Idx, std::tuple<RemoveCVREF<TupleTypes>...>>;
	};

	template<typename, typename> struct AddMemberToTuple;

	template<typename NewTupleMember, typename... CurrentTupleMember>
	struct AddMemberToTuple<NewTupleMember, std::tuple<CurrentTupleMember...>> {
		using Type = std::tuple<CurrentTupleMember..., NewTupleMember>;
	};

	template<typename InputTupleType, typename OutputTupleType>
	using AddToTupleIfNotExist = std::conditional_t < TupleHasType<InputTupleType, OutputTupleType>::value,
		OutputTupleType, typename AddMemberToTuple<InputTupleType, OutputTupleType>::Type > ;


	template<typename OutputTupleType, typename FirstInputTupleType, typename... RestInputTupleType>
	struct TupleTypeFilter {
		using Type = typename TupleTypeFilter<AddToTupleIfNotExist<FirstInputTupleType, OutputTupleType>, RestInputTupleType...>::Type;
	};

	template<typename OutputTupleType, typename LastInputTupleType>
	struct TupleTypeFilter<OutputTupleType, LastInputTupleType> {
		using Type = AddToTupleIfNotExist<LastInputTupleType, OutputTupleType>;
	};

	template<typename ToCountType, std::size_t CurrentCounter, typename FirstTupleType, typename... RestInputTupleTypes>
	struct TupleTypeCounter {
		static constexpr std::size_t Count = (std::is_same<ToCountType, FirstTupleType>::value) ?
			TupleTypeCounter<ToCountType, CurrentCounter + 1, RestInputTupleTypes...>::Count :
			TupleTypeCounter<ToCountType, CurrentCounter, RestInputTupleTypes...>::Count;
	};

	template<typename ToCountType, std::size_t CurrentCounter, typename LastTupleType>
	struct TupleTypeCounter<ToCountType, CurrentCounter, LastTupleType> {
		static constexpr std::size_t Count = (std::is_same<ToCountType, LastTupleType>::value) ?
			CurrentCounter + 1 : CurrentCounter;
	};

	template<typename, typename> struct CountTypeInTuple;

	template<typename ToCountType, typename... TupleMember>
	struct CountTypeInTuple<ToCountType, std::tuple<TupleMember...>> {
		static constexpr std::size_t Count = 
			TupleTypeCounter<std::remove_reference_t<std::remove_cv_t<ToCountType>>, 0, TupleMember...>::Count;
	};

	template<typename... TupleMember>
	constexpr auto CreateFilteredTuple(const std::tuple<TupleMember...>& tuple) {
		(void)tuple;
		return typename TupleTypeFilter<std::tuple<>, TupleMember...>::Type{};
	}

	template<typename, typename, typename> struct CreateFilterdArrayTuple;

	template<typename UnchangedTuple, typename NewTupleType> 
	struct CreateFilterdArrayTuple < UnchangedTuple, NewTupleType, std::tuple<>> {
		using Type = NewTupleType;
	};

	template<typename UnchangedTuple, typename NewTupleType, typename FirstTupleMember, typename...RestTupleMember> 
	struct CreateFilterdArrayTuple<UnchangedTuple, NewTupleType, std::tuple <FirstTupleMember, RestTupleMember... >> {
		using ArrayType = std::array<FirstTupleMember, CountTypeInTuple<FirstTupleMember, UnchangedTuple>::Count>;
		using Type = typename CreateFilterdArrayTuple<
			UnchangedTuple, AddToTupleIfNotExist<ArrayType, NewTupleType>, std::tuple<RestTupleMember...>
		>::Type;
	};

	template <typename, typename> struct TupleIndex;

	template <typename T, typename... Types>
	struct TupleIndex<T, std::tuple<T, Types...>> {
		static const std::size_t Index = 0;
	};

	template <typename T, typename U, typename... Types>
	struct TupleIndex<T, std::tuple<U, Types...>> {
		static const std::size_t Index = 1 + TupleIndex<T, std::tuple<Types...>>::Index;
	};

	template<typename,typename> struct AddTypeToTuple;

	template<typename NewType, typename... TupleTypes> 
	struct AddTypeToTuple<NewType, std::tuple<TupleTypes...>> {
		using Type = std::tuple<NewType, TupleTypes...>;
	};

	template<std::size_t Index, typename... OldIntegralConstantTupleTypes>
	constexpr decltype(auto) DoAddIntegralConstantDoTuple(const std::tuple<OldIntegralConstantTupleTypes...>& dummyTuple) {
		(void)dummyTuple;
		return std::tuple<OldIntegralConstantTupleTypes..., std::integral_constant<std::size_t, Index>>{};
	}

	
}//namespace INTERNAL
}//namepsace messaging

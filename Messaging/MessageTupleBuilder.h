#pragma once
#include <cstddef>
#include <tuple>
#include <array>
#include "MessagingTupleUtils.h"

namespace messaging {
namespace INTERNAL {

	template<typename, typename, typename...> struct MessageTupleBuilder;
	
	template<typename ToAddType, typename UnchangedTupleType, typename CurrentTupleType>
	using AddArrayToTuple = decltype(AddMemberToVariadicTemplate < 
		std::array<std::remove_reference_t<std::remove_cv_t<ToAddType>>,
		CountTypeInTuple<ToAddType, UnchangedTupleType>::Count >>
	(CurrentTupleType{}));

	template<typename UnchangedTupleType, typename CurrentTupleType, typename LastTupleType>
	struct MessageTupleBuilder<UnchangedTupleType, CurrentTupleType, std::tuple<LastTupleType>> {
		using Type = AddArrayToTuple<LastTupleType, UnchangedTupleType, CurrentTupleType>;
	};

	template<typename UnchangedTupleType, 
		typename CurrentTupleType,
		typename FirstTupleType, 
		typename... RestTupleTypes>
	struct MessageTupleBuilder <
		UnchangedTupleType,
		CurrentTupleType,
		std::tuple<FirstTupleType, RestTupleTypes...> > {
		using Type = typename MessageTupleBuilder<UnchangedTupleType, 
			AddArrayToTuple<FirstTupleType, UnchangedTupleType, CurrentTupleType>, 
		std::tuple<RestTupleTypes...>>::Type;
	};
	
	template<template<typename> class Trait, typename> struct DoTraitOnEachTupleMember;

	template<template<typename> class Trait, typename... TupleTypes> 
	struct DoTraitOnEachTupleMember<Trait, std::tuple<TupleTypes...>> {
		using Type = std::tuple<Trait<TupleTypes>...>;
	};
}//namespace INTERNAL
}//namepsace messaging
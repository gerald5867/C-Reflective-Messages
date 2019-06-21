#pragma once
#include <sstream>
#include <type_traits>
#include <initializer_list>
#include "IMessage.h"
#include "MessageIndiceBuilder.h"
#include "MessageHelpers.h"
#include "MessagingTupleUtils.h"
#include "MessageTupleBuilder.h"

namespace messaging {
namespace INTERNAL {

	class BinaryDeserializer;
	class BinarySerializer;
}
	template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
	class BasicMessage : public IMessage
	{
	public:
		using TupleFieldTypes = std::tuple<BasicMessageFieldTypes...>;
	protected:
		using FilterdTupleType = typename INTERNAL::TupleTypeFilter<std::tuple<>, BasicMessageFieldTypes...>::Type;
		using BuildedTupleType = typename INTERNAL::MessageTupleBuilder<std::tuple<BasicMessageFieldTypes...>,
			std::tuple<>, FilterdTupleType>::Type;

		using BasicMessageType = BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>;

		template<typename, typename...> friend class BasicMessage;
		template<typename...> friend class CombinedMessage;
		friend INTERNAL::BinarySerializer;
		friend INTERNAL::BinaryDeserializer;

		BuildedTupleType m_memberFields;
		
		template<typename FieldType>
		constexpr decltype(auto) GetArrayFields() const;
		template<typename FieldType>
		constexpr decltype(auto) GetArrayFields();

		constexpr decltype(auto) GetAllArrayFields() const;

	public:
		//Todo replace std::size_t with real Idx Type
		template<std::size_t Idx, typename ValueType>
		void SetOne(ValueType&& value);

		template<typename... InitVarArgTypes>
		void SetAll(InitVarArgTypes&&... fields);

		template<std::size_t Idx>
		decltype(auto) GetOne();
		template<std::size_t Idx>
		decltype(auto) GetOne() const;

		constexpr static std::size_t GetStaticFieldCount() noexcept;
		constexpr static std::size_t GetStaticMessageSize() noexcept {
			std::size_t res = 0;
			(void)std::initializer_list<int>{(res += INTERNAL::SizeOfMessageField<BasicMessageFieldTypes, void> ::Size, 0)...};
			return res;
		}

		static constexpr std::size_t FieldCount = GetStaticFieldCount();

		template<typename PredType>
		constexpr void ForEachField(PredType&& pred) const;
		template<typename PredType>
		constexpr void ForEachField(PredType&& pred);

		virtual std::size_t GetMessageSize() const noexcept override;

		bool operator == (const BasicMessageType& other) const;
		bool operator != (const BasicMessageType& other) const;

	public:

		template<typename... InitVarArgTypes, 
			std::enable_if_t<(!std::is_base_of<BasicMessageType, 
			typename INTERNAL::TupleTypeAt<0, std::tuple<InitVarArgTypes...>>::Type>::value
			&& sizeof...(InitVarArgTypes) != 0)
		, bool> Dummy = false>
		BasicMessage(InitVarArgTypes&&... fields);
		BasicMessage() = default;

	protected:
		~BasicMessage() noexcept = default;
		BasicMessage(const BasicMessage&) = default;
		BasicMessage& operator=(const BasicMessage&) = default;
		BasicMessage(BasicMessage&&) = default;
		BasicMessage& operator=(BasicMessage&&) = default;

		virtual bool IsEqual(const IMessage&) const override;
	private:
		template<typename PredType, std::size_t... Indices>
		static constexpr void ForEachFieldHelper(BasicMessageType& msg, std::index_sequence<Indices...>, PredType&& pred);

		template<typename TupleType, std::size_t... Indices>
		decltype(auto) InitVarArgs(TupleType&& tupleArgs, std::index_sequence<Indices...>);
		std::size_t InternalGetMessageSize() const noexcept;

		template<typename PredicateType>
		constexpr void ForEachArrayFieldDo(PredicateType&& predicate);
		template<typename PredicateType>
		constexpr void ForEachArrayFieldDo(PredicateType&& predicate) const;
 
		template<typename MessageType, typename...TupleTypes, typename PredicateType>
		static constexpr inline void InternalForEachArrayFieldDo(MessageType&& msg,
			const std::tuple<TupleTypes...>& dummyTuple, PredicateType&& predicate);
	
	};


	template<typename DerivedMessageType>
	class BasicMessage<DerivedMessageType> : public IMessage {
	public:
		constexpr static std::size_t GetStaticMessageSize() noexcept { return 0; }
		constexpr static std::size_t GetStaticFieldCount() noexcept { return 0; }
		virtual std::size_t GetMessageSize() const noexcept override final { return 0; }
		template<typename PredicateType>
		constexpr inline void ForEachArrayFieldDo(PredicateType&& predicate) const noexcept { (void)predicate; }
		template<typename PredType>
		constexpr void ForEachField(PredType&& pred) const noexcept { (void)pred; }
	};
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename FieldType>
constexpr decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetArrayFields() const {
	using Type = typename INTERNAL::EnumConverter<FieldType>::Type;
	static constexpr std::size_t Count = INTERNAL::CountTypeInTuple<Type, TupleFieldTypes>::Count;
	return std::get<std::array<Type, Count>>(m_memberFields);
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
constexpr decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetAllArrayFields() const {
	return m_memberFields;
}

template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename FieldType>
constexpr decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetArrayFields() {
	using Type = INTERNAL::RemoveCVREF<decltype(const_cast<const BasicMessageType*>(this)->template GetArrayFields<FieldType>())>;
	return const_cast<Type&>(const_cast<const BasicMessageType*>(this)->template GetArrayFields<FieldType>());
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<std::size_t Idx, typename ValueType>
void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::SetOne(ValueType&& value) {
	using DecayedType = INTERNAL::RemoveCVREF<ValueType>;
	using Type = std::tuple_element_t<Idx, TupleFieldTypes>;
	using IndiceContainerType = typename INTERNAL::CreateIndicesByTupleType<Type, TupleFieldTypes>::Type;
	static constexpr decltype(auto) ConvertedIndiceContainer = INTERNAL::CreateIndiceContainerZeroOffset(IndiceContainerType{});
	static constexpr std::size_t Count = INTERNAL::CountTypeInTuple<Type, TupleFieldTypes>::Count;
	std::get<std::array<Type, Count>>(m_memberFields)[ConvertedIndiceContainer[Idx]] = std::forward<ValueType>(value);
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<std::size_t Idx>
decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetOne() {
	using Type = std::tuple_element_t<Idx, TupleFieldTypes>;
	static constexpr std::size_t Count = INTERNAL::CountTypeInTuple<Type, TupleFieldTypes>::Count;
	using IndiceContainerType = typename INTERNAL::CreateIndicesByTupleType<Type, TupleFieldTypes>::Type;
	static constexpr decltype(auto) ConvertedIndiceContainer = INTERNAL::CreateIndiceContainerZeroOffset(IndiceContainerType{});
	static_assert(Idx < ConvertedIndiceContainer.size(), R"(you probably did provide the wrong data type because 
				there are no Field with that index and data type you specified!!!!!)");
	return std::get<std::array<Type, Count>>(m_memberFields)[ConvertedIndiceContainer[Idx]];
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<std::size_t Idx>
decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetOne() const {
	using Type = INTERNAL::RemoveCVREF<decltype(const_cast<BasicMessageType*>(this)->GetOne<Idx>())>;
	return const_cast<const Type&>(const_cast<BasicMessageType*>(this)->GetOne<Idx>());
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
constexpr std::size_t messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetStaticFieldCount() noexcept {
	return sizeof...(BasicMessageFieldTypes);
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename PredType>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::ForEachField(PredType&& pred) const {
	ForEachFieldHelper(
		const_cast<BasicMessageType&>(*this),
		std::make_index_sequence<sizeof...(BasicMessageFieldTypes)>{}, std::forward<PredType>(pred));
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename PredType>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::ForEachField(PredType&& pred) {
	const_cast<const BasicMessageType*>(this)->ForEachField(std::forward<PredType>(pred));
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
inline std::size_t messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::GetMessageSize() const noexcept {
	return InternalGetMessageSize();
}

template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename... InitVarArgTypes,
	std::enable_if_t<(!std::is_base_of<messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>,
		typename messaging::INTERNAL::TupleTypeAt<0, std::tuple<InitVarArgTypes...>>::Type>::value
		&& sizeof...(InitVarArgTypes) != 0)
	, bool> Dummy>
messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::BasicMessage(InitVarArgTypes&&... fields) {
	InitVarArgs(std::forward_as_tuple(fields...), std::make_index_sequence<sizeof...(InitVarArgTypes)>{});
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
bool messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::operator == (const BasicMessageType& other) const {
	return GetAllArrayFields() == other.GetAllArrayFields();
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
inline bool messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::operator != (const BasicMessageType& other) const {
	return !(*this == other);
}


template<typename DerivedMessageType, typename ...BasicMessageFieldTypes>
inline bool messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::IsEqual(const IMessage& other) const {
	return dynamic_cast<const BasicMessageType&>(other) == *this;
}


template<typename DerivedMessageType, typename ...BasicMessageFieldTypes>
inline std::size_t messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::InternalGetMessageSize() const noexcept {
	std::size_t res = 0;
	ForEachArrayFieldDo([&res](const auto& val, const std::size_t Idx) { return res += INTERNAL::DynamicSizeOfMessageField(val); });
	return res;
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename... InitVarArgTypes>
inline void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::SetAll(InitVarArgTypes&&... fields) {
	InitVarArgs(std::forward_as_tuple(std::forward<InitVarArgTypes>(fields)...), std::make_index_sequence<sizeof...(InitVarArgTypes)>{});
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename PredType, std::size_t... Indices>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::ForEachFieldHelper(BasicMessageType& msg, std::index_sequence<Indices...>, PredType&& pred) {
	(void)std::initializer_list<int>{(pred(msg.GetOne<Indices>(), Indices), 0)...};
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename TupleType, std::size_t... Indices>
decltype(auto) messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::InitVarArgs(TupleType&& tupleArgs, std::index_sequence<Indices...>) {
	const auto InitLambda = [this](auto&& val, auto& fieldRef) {
		static_assert(std::is_convertible<INTERNAL::RemoveCVREF<decltype(val)>, INTERNAL::RemoveCVREF<decltype(fieldRef)>>::value,
			"One or More Arguments from SetAll/Ctor are not convertible to the appropriated member fields!!!");
		fieldRef = std::forward<decltype(val)>(val);
	};
	(void)std::initializer_list<int>{ (InitLambda(
		std::forward<std::tuple_element_t<Indices, TupleType>>(std::get<Indices>(tupleArgs)),
		GetOne<Indices>()
	), 0)... };
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename PredicateType>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::ForEachArrayFieldDo(PredicateType&& predicate) {
	const_cast<const BasicMessageType*>(this)->ForEachArrayFieldDo(std::forward<PredicateType>(predicate));
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename PredicateType>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::ForEachArrayFieldDo(PredicateType&& predicate) const {
	InternalForEachArrayFieldDo(*this, FilterdTupleType{}, std::forward<PredicateType>(predicate));
}


template<typename DerivedMessageType, typename... BasicMessageFieldTypes>
template<typename MessageType, typename...TupleTypes, typename PredicateType>
constexpr void messaging::BasicMessage<DerivedMessageType, BasicMessageFieldTypes...>::InternalForEachArrayFieldDo(MessageType&& msg,
	const std::tuple<TupleTypes...>& dummyTuple, PredicateType&& predicate) {
	(void)dummyTuple;
	using TupleT = std::tuple<INTERNAL::RemoveCVREF<TupleTypes>...>;
	(void)std::initializer_list<int>{(predicate(
		const_cast<INTERNAL::RemoveCVREF<MessageType>&>(msg).template GetArrayFields<TupleTypes>(),
		INTERNAL::TupleIndex<TupleTypes, TupleT>::Index), 0)...};
}
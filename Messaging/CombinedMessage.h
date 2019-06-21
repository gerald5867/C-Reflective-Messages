#pragma once
#include <algorithm>
#include <type_traits>
#include <memory>
#include "Message.h"
namespace messaging {


	template<typename...DerivedBasicMessageTypes>
	class CombinedMessage final : public virtual DerivedBasicMessageTypes..., public virtual IMessage {
	public:
		using MessageBaseClassTuple = std::tuple<DerivedBasicMessageTypes...>;
		using MyType = CombinedMessage<DerivedBasicMessageTypes...>;

		virtual std::unique_ptr<IMessage> Clone() const override;
		virtual std::size_t GetMessageSize() const noexcept override final;

		static constexpr std::size_t GetStaticFieldCount() noexcept;
		static constexpr std::size_t GetStaticMessageSize() noexcept;

		template<std::size_t Idx>
		decltype(auto) GetOne();
		template<std::size_t Idx>
		decltype(auto) GetOne() const;

		template<std::size_t Idx, typename ValueType>
		void SetOne(ValueType&& value);

		template<typename... FieldTypes>
		void SetAll(FieldTypes&&... fields);

		template<typename PredType>
		constexpr void ForEachField(PredType&& pred) const;
		template<typename PredType>
		constexpr void ForEachField(PredType&& pred);

		bool operator != (const MyType& other) const;
		bool operator == (const MyType& other) const;
	private:
		template<typename PredType, std::size_t... Indices>
		constexpr void ForEachFieldHelper(std::index_sequence<Indices...>, PredType&& pred);

		template<typename MessageType, std::size_t Idx>
		struct MessageTypeInformation {
			static constexpr std::size_t RealIndex = Idx;
			using BaseType = MessageType;
			constexpr MessageTypeInformation() = default;
			constexpr MessageTypeInformation(const MessageTypeInformation&) = default;
		};
		template<typename PredType>
		static constexpr void ForEachMessageBaseDo(CombinedMessage<DerivedBasicMessageTypes...>* inst, PredType&& pred);
		static constexpr std::size_t GetMaxFieldCount();
		template<std::size_t Idx>
		static constexpr decltype(auto) CreateTypeInformation();
	};
}


template<typename... DerivedBasicMessageTypes>
inline std::unique_ptr<messaging::IMessage> messaging::CombinedMessage<DerivedBasicMessageTypes...>::Clone() const {
	return std::make_unique<CombinedMessage>();
}


template<typename ...DerivedBasicMessageTypes>
inline std::size_t messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetMessageSize() const noexcept {
	std::size_t sum = 0;
	ForEachMessageBaseDo(const_cast<MyType*>(this), [&sum](const auto& msg) { sum += msg->InternalGetMessageSize(); });
	return sum;
}


template<typename ...DerivedBasicMessageTypes>
inline constexpr std::size_t messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetStaticFieldCount() noexcept {
	std::size_t res = 0;
	ForEachMessageBaseDo(nullptr, [&res](const auto& msg) { res += msg->GetStaticFieldCount(); });
	return res;
}

template<typename ...DerivedBasicMessageTypes>
inline constexpr std::size_t messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetStaticMessageSize() noexcept {
	std::size_t res = 0;
	ForEachMessageBaseDo(nullptr, [&res](const auto& msg) { res += msg->GetStaticMessageSize(); });
	return res;
}


template<typename ...DerivedBasicMessageTypes>
inline bool messaging::CombinedMessage<DerivedBasicMessageTypes...>::operator!=(const MyType & other) const {
	return !(*this == other);
}


template<typename ...DerivedBasicMessageTypes>
inline bool messaging::CombinedMessage<DerivedBasicMessageTypes...>::operator==(const MyType& other) const {
	const IMessage* baseClassPtrs[] = { static_cast<const IMessage*>(static_cast<const DerivedBasicMessageTypes*>(this))... };
	const IMessage* otherBaseClassPtrs[] = { static_cast<const IMessage*>(static_cast<const DerivedBasicMessageTypes*>(&other))... };
	for (std::size_t i = 0; i < sizeof...(DerivedBasicMessageTypes); ++i) {
		if (!baseClassPtrs[i]->IsEqual(*(otherBaseClassPtrs[i]))) {
			return false;
		}
	}
	return true;
}


template<typename ...DerivedBasicMessageTypes>
inline constexpr std::size_t messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetMaxFieldCount() {
	std::size_t res = 0;
	ForEachMessageBaseDo(nullptr, [&res](const auto& msg) {
		res = (std::max)(res, msg->GetStaticFieldCount());
	});
	return res;
}


template<std::size_t N>
inline constexpr std::pair<std::size_t, std::size_t> GetStepIndex(const std::size_t(&FieldCounts)[N], const std::size_t Idx) {
	std::size_t curFieldIndex = 0;
	for (std::size_t i = 0; i < N; ++i) {
		curFieldIndex += FieldCounts[i];
		if (Idx < curFieldIndex) {
			// 1, 0 ?? wenn idx == 2 dann 0 wenn idx == 3 dann 1
			return std::make_pair(i , Idx - (curFieldIndex - FieldCounts[i]));
		}
	}
	return std::make_pair((std::size_t) -1, (std::size_t) -1);
}


template<typename ...DerivedBasicMessageTypes>
template<std::size_t Idx> 
inline constexpr decltype(auto) messaging::CombinedMessage<DerivedBasicMessageTypes...>::CreateTypeInformation() {
	static constexpr std::size_t FieldCounts[] = { DerivedBasicMessageTypes::GetStaticFieldCount()... };
	static constexpr auto IndexAndSubstractCount = GetStepIndex(FieldCounts, Idx);
	using BaseType = std::tuple_element_t<IndexAndSubstractCount.first, MessageBaseClassTuple>;
	return MessageTypeInformation<BaseType, IndexAndSubstractCount.second>{};
}

namespace messaging {
namespace INTERNAL {
	template<std::size_t Idx, typename MessageType, bool Test = std::less{}(Idx,  MessageType::GetStaticFieldCount()),
		std::enable_if_t<Test, bool> Dummy = false>
	inline decltype(auto) GetOneIfLess(MessageType& msg) {
		return msg.template GetOne<Idx>();
	}

	template<std::size_t Idx, typename MessageType, bool Test = std::less{}(Idx, MessageType::GetStaticFieldCount()),
		std::enable_if_t<!Test, bool> Dummy = false>
	inline int& GetOneIfLess(MessageType& msg) {
		static int dummy = 0;
		throw std::runtime_error("THIS SHOULD NEVER HAPPEN!!");
		return dummy;
	}

}
}


template<typename ...DerivedBasicMessageTypes>
template<std::size_t Idx>
inline decltype(auto) messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetOne() {
	using TypeInfo = decltype(CreateTypeInformation<Idx>());
	return messaging::INTERNAL::GetOneIfLess<TypeInfo::RealIndex>(static_cast<typename TypeInfo::BaseType&>(*this));
}


template<typename ...DerivedBasicMessageTypes>
template<std::size_t Idx>
inline decltype(auto) messaging::CombinedMessage<DerivedBasicMessageTypes...>::GetOne() const {
	using TypeInfo = decltype(CreateTypeInformation<Idx>());
	return messaging::INTERNAL::GetOneIfLess<TypeInfo::RealIndex>(static_cast<const typename TypeInfo::BaseType&>(*this));
}


template<typename ...DerivedBasicMessageTypes>
template<std::size_t Idx, typename ValueType>
inline void messaging::CombinedMessage<DerivedBasicMessageTypes...>::SetOne(ValueType&& value) {
	using TypeInfo = decltype(CreateTypeInformation<Idx>());
	static_cast<typename TypeInfo::BaseType&>(*this).template SetOne<TypeInfo::RealIndex>(std::forward<ValueType>(value));
}

template<typename ...DerivedBasicMessageTypes>
template<typename ...FieldTypes>
inline void messaging::CombinedMessage<DerivedBasicMessageTypes...>::SetAll(FieldTypes&&... fields) {
	throw std::runtime_error{ "Not implemented yet!!!" };
}


template<typename ...DerivedBasicMessageTypes>
template<typename PredType>
inline constexpr void messaging::CombinedMessage<DerivedBasicMessageTypes...>::ForEachField(PredType&& pred) const {
	const_cast<messaging::CombinedMessage<DerivedBasicMessageTypes...>*>(this)->ForEachField(std::forward<PredType>(pred));
}


template<typename ...DerivedBasicMessageTypes>
template<typename PredType>
inline constexpr void messaging::CombinedMessage<DerivedBasicMessageTypes...>::ForEachField(PredType&& pred) {
	ForEachFieldHelper(std::make_index_sequence<GetStaticFieldCount()>(), std::forward<PredType>(pred));
}


template<typename ...DerivedBasicMessageTypes>
template<typename PredType, std::size_t...Indices>
constexpr void messaging::CombinedMessage<DerivedBasicMessageTypes...>::ForEachFieldHelper(
std::index_sequence<Indices...>, PredType&& pred) {
	(void)std::initializer_list<int>{
		(pred(GetOne<Indices>(), Indices), 0)...
	};
}


template<typename ...DerivedBasicMessageTypes>
template<typename PredType>
inline constexpr void messaging::CombinedMessage<DerivedBasicMessageTypes...>::ForEachMessageBaseDo(
	CombinedMessage<DerivedBasicMessageTypes...>* inst, PredType&& pred) {
	(void)std::initializer_list<int>{
		(pred(static_cast<DerivedBasicMessageTypes*>(inst)), 0)...
	};
}
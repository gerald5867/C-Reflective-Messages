#pragma once
#include <cstdint>
#include <limits>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "BasicMessage.h"
#include "ExtendedEnum.h"
#include "MessageHelpers.h"

#define DECLSTATICARRAY(type, len)  decltype(std::array<type, len>{})
#define DECLDYNAMICARRAY(type) std::vector<type>

#define EXPANDCREATEFIELD(r, token, i, e) token(e, i)
#define FOREACHFIELDNAME(token, ...) BOOST_PP_SEQ_FOR_EACH_I(EXPANDCREATEFIELD, token, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define GENERATE_FIELD(name, index) \
	using BOOST_PP_CAT(name, Type) = std::tuple_element_t<index, TupleFieldTypes>; \
	using BOOST_PP_CAT(name, ConstFieldType) = std::conditional_t< \
		std::is_arithmetic<BOOST_PP_CAT(name, Type)>::value, BOOST_PP_CAT(name, Type), const BOOST_PP_CAT(name, Type) &>;\
	BOOST_PP_CAT(name, ConstFieldType) BOOST_PP_CAT(Get, name()) const noexcept { return this->GetOne<index>(); } \
	BOOST_PP_CAT(name, Type&) BOOST_PP_CAT(Get, name()) noexcept { return this->GetOne<index>(); } \
	template<typename FieldType> \
	void BOOST_PP_CAT(Set, name)(FieldType&& value) { return this->SetOne<index>(std::forward<FieldType>(value)); }


namespace messaging {
	class EmptyMessage : public BasicMessage<EmptyMessage>{
	public:
		virtual ~EmptyMessage() noexcept = default;
		virtual std::unique_ptr<IMessage> Clone() const override { return std::unique_ptr<IMessage>(new EmptyMessage(*this)); }
	};
}

#define DECLFIELDNAMES(...) \
	FOREACHFIELDNAME(GENERATE_FIELD, __VA_ARGS__) \
	DECLENUMEX(FieldName, std::size_t, __VA_ARGS__) \
	static constexpr decltype(auto) FieldNameStrings = FieldName::EnumStrings; \
	static constexpr decltype(auto) FieldNameLens = messaging::INTERNAL::CreateMessageKeyLens(FieldNameStrings);

#define TYPE_FROM_FIELD_SEQUENCE(r, argument, i, e) BOOST_PP_SEQ_ELEM(0, e) BOOST_PP_COMMA_IF(BOOST_PP_GREATER(argument, BOOST_PP_ADD(i, 1)))
#define	NAME_FROM_FIELD_SEQUENCE(r, argument, i, e) BOOST_PP_SEQ_ELEM(1, e) BOOST_PP_COMMA_IF(BOOST_PP_GREATER(argument, BOOST_PP_ADD(i, 1)))
#define FOREACH_FIELD_DO(argument, doMacro, ...) BOOST_PP_SEQ_FOR_EACH_I(doMacro, argument, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define CREATE_TYPECOMMALIST_FROM_VARARGS(...) FOREACH_FIELD_DO(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), TYPE_FROM_FIELD_SEQUENCE, __VA_ARGS__)
#define CREATE_NAMECOMMALIST_FROM_VARARGS(...) FOREACH_FIELD_DO(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__) , NAME_FROM_FIELD_SEQUENCE, __VA_ARGS__)


#ifndef DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE
	#define DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE 0
#endif
#if DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE
	#define DECLMESSAGE_DECLARE_EXTERN_TEMPLATE(...) extern template class __VA_ARGS__;
#else
	#define DECLMESSAGE_DECLARE_EXTERN_TEMPLATE(...)
#endif


#ifndef DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION
	#define DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION 0
#endif
#if DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION
	#define DECLMESSAGE_EXPLICIT_TEMPLATE_INSTANTATION(...)
#else
	#define DECLMESSAGE_EXPLICIT_TEMPLATE_INSTANTATION(...) template class __VA_ARGS__;
#endif


#define DECLMESSAGEFIELD(...) BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)
#define DECLMESSAGE(msgName, ...) \
	class msgName : public messaging::BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)> { \
	public:\
		static constexpr const char MessageStringName[] = #msgName; \
		using MyBaseType = messaging::BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>; \
		using FieldTypes = std::tuple<CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>; \
		using BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>::BasicMessage;\
		using BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>::operator =; \
		virtual ~msgName() noexcept = default; \
		virtual std::unique_ptr<IMessage> Clone() const override { \
			return std::unique_ptr<IMessage>(new msgName(*this)); \
		} \
		DECLFIELDNAMES(CREATE_NAMECOMMALIST_FROM_VARARGS(__VA_ARGS__)) \
	}; \
	DECLMESSAGE_DECLARE_EXTERN_TEMPLATE(messaging::BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>) \
	DECLMESSAGE_EXPLICIT_TEMPLATE_INSTANTATION(messaging::BasicMessage<msgName, CREATE_TYPECOMMALIST_FROM_VARARGS(__VA_ARGS__)>)

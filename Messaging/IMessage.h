#pragma once
#include <type_traits>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include "MessageHelpers.h"

namespace messaging {
	
	class MessageFieldException final : public std::exception {
	public:
		template<typename T>
		MessageFieldException(T dummy, const bool IsArray = false) {
			(void)dummy;
			if (!IsArray) {
				m_message = std::string{ "Message does not contain a field of type " } +typeid(T).name();
			} else {
				m_message = std::string{ "Message does not contain a array field of type " } +typeid(T).name();
			}
		}
		virtual const char* what() const override { return m_message.c_str(); }
	private:
		std::string m_message;
	};

	class IMessage {
	public:
		virtual ~IMessage() noexcept = default;
		virtual std::unique_ptr<IMessage> Clone() const = 0;
		virtual std::size_t GetMessageSize() const noexcept = 0;
		virtual bool IsEqual(const IMessage&) const { throw std::runtime_error{"not implemented!!!"}; }

	};
}
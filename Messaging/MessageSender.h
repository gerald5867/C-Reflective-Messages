#pragma once
#include "../stdafx.h"
#include "../char_manager.h"
#include "../p2p.h"
#include "../char.h"
#include "../desc.h"
#include "../Messaging/MessageBinarySerializer.h"
#include "BasicMessage.h"
namespace messaging {
class MessageSender final {
public:
	template<typename DerivedType, typename... MessageTypes>
	static bool SendMessage(CHARACTER* pChar, const BasicMessage<DerivedType, MessageTypes...>& toSendMsg) {
		if (pChar == nullptr || pChar->GetDesc() == nullptr || (!pChar->IsPC())) {
			return false;
		}
		const std::vector<Byte>& byteVec = binary_serilization::Serialize(toSendMsg);
		pChar->GetDesc()->Packet(byteVec.data(), byteVec.size());
		return true;
	}


	template<typename DerivedType, typename... MessageTypes>
	static void SendMessageToEachCharacter(const BasicMessage<DerivedType, MessageTypes...>& toSendMsg) {

	}


	template<typename DerivedType, typename... MessageTypes>
	static void SendMessageToEachGameProcess(const BasicMessage<DerivedType, MessageTypes...>& toSendMsg) {

	}
};
}
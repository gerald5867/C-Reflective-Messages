#include <iostream>
#include <vector>
#include <fstream>
#include "Reflective_Messages.h"

DECLMESSAGE(TestMessage,
	DECLMESSAGEFIELD(int, Age),
	DECLMESSAGEFIELD(std::string, Name),
	DECLMESSAGEFIELD(std::string, Country)
);


DECLMESSAGE(SecondTestMessage,
	DECLMESSAGEFIELD(std::vector<int>, Numbers)
)


//combines both messages to one
using TestMessageWithNumbers = messaging::CombinedMessage<TestMessage, SecondTestMessage>;


void PrintTestMessage(const TestMessage& msg) {
	//Iterating over all fields with a lambda.
	//First paramater : field starting with Age
	//Second parameter : index of the field starting with 0
	msg.ForEachField([](const auto& field, auto idx) {
		std::cout << "field number " << idx << " " << TestMessage::FieldNameStrings[idx].data() << " : " << field << "\n";
		});
}


int main() {
	TestMessage msg;
	msg.SetAge(23);
	msg.SetName("Gerald");
	msg.SetCountry("Austria");
	PrintTestMessage(msg);

	//the combined message it inherits all the messages 
	TestMessageWithNumbers numMsg;
	numMsg.GetNumbers().emplace_back(5);
	numMsg.SetAge(23);

	//Getting the current size In bytes of the message at runtime
	//If the message size is known at compile time you can retrive it with GetStaticMessageSize.
	//Otherwise a static_assert will fail in GetStaticMessageSize
	std::cout << "message current size in bytes : " << msg.GetMessageSize() << "\n";
	std::cout << std::flush;


	//Getting the age again with the GetOne Function that works like std::get
	std::cout << "age : " << msg.template GetOne<0>() << std::endl;

	//setting the fields again so you can see the difference after loading the message from the file
	msg.SetAge(50);
	msg.SetName("Merlin");
	msg.SetCountry("Germany");
	//to binary file
	std::vector<messaging::Byte> serializedContent = messaging::binary_serilization::Serialize(msg);
	std::ofstream outFile;
	outFile.open("testmsg.msg", std::ios::out | std::ios::trunc | std::ios::binary);
	if (outFile.is_open()) {
		outFile.write(reinterpret_cast<const char*>(serializedContent.data()), serializedContent.size());
		outFile.close();
	}

	//from binary file
	std::ifstream inFile;
	inFile.open("testmsg.msg", std::ios::in | std::ios::binary);
	if (inFile.is_open()) {
		inFile.seekg(0, std::ios::end);
		serializedContent.clear();
		serializedContent.resize(static_cast<std::size_t>(inFile.tellg()));
		inFile.seekg(0, std::ios::beg);
		inFile.read(reinterpret_cast<char*>(serializedContent.data()), serializedContent.size());
		messaging::binary_serilization::Deserialize(msg, serializedContent);
		PrintTestMessage(msg);
	}

	//if you are using the nlohmann json lib you can use my JSON Serializer/Deserializer also
	//it works almost the same as the Binary Serializer/Deserializer
	return 0;
}
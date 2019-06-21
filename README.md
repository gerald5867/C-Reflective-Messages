# C-Reflective-Messages
A header only module to create messages that have full Informations about all their fields in C++ using template and preprocessor meta programming.

Reflective-Messages is an high performance C++ library that uses C++14 + template meta programming and preprocessor meta programming to create messages that support reflection.
For Example: Having an message with 10 different named int fields the message holds all 10 in an contiguous allowing it to memcpy all 10 ints in one go when it comes to serilization.

---
My use cases for these messages:
* MySQL tables and rows.
* TCP message passing.
* Message passing from and to python using my own generic python exporter for these messages.
* Create a JSON File from it and parsing it from a JSON file.

Compile time is currently a little bit slow but this may change in the future.
To improve the compile time you can use the module like this:

in Message.h Change DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE to 1
.. code:: c++
  #ifndef DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE
    #define DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE 1
  #endif

declare the message in the header file like you normally would do : 

.. code:: c++
  DECLMESSAGE(TestMessage,
    DECLMESSAGEFIELD(int, Age),
    DECLMESSAGEFIELD(std::string, Name),
    DECLMESSAGEFIELD(std::string, Country)
  );
  
 and do the following in the cpp file :
.. code:: c++

  #pragma push_macro("DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE")
  #pragma push_macro("DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION")

  #undef DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE
  #undef DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION

  #define DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE 0
  #define DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION 1

  #include "myheaderfile.h"

  #pragma pop_macro("DECLMESSAGE_ALLOW_DECLARE_EXTERN_TEMPLATE")
  #pragma pop_macro("DECLMESSAGE_ALLOW_EXPLICIT_TEMPLATE_INSTANTATION")

now you have extern template in the header file an and explicit instantation in the cpp file for all messages in
that header file automatically.

The following example are also in main.cpp:

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

	//the JSON Serializer/Deseriliazer uses the nlohmann JSON lib see : https://github.com/nlohmann/json
	//It works almost the same as the Binary Serializer/Deserializer
	return 0;
}

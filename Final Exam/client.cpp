#include <iostream>
#include <string>
#include "socket.h"

using namespace std;

int main() {

	data_packet my_packet = create_packet();

	//these lines are useful to test your serializer and deserializer
	//without requiring to run two applications

	struct serialized_packet serialized_data = data_packet_serializer(my_packet);

	data_packet recovered = data_packet_deserializer(serialized_data.data);

	cout << endl;
	cout << "Client packet: " <<  my_packet.data << endl;
	cout << "Serialized data: " << serialized_data.data << endl;
	cout << "Deserialized data: " << recovered.data << endl << endl;
	
	cout << "CRC output (Client data packet -- Client side): ";
	display(my_packet.crc);

	cout << "CRC output (Deserialized Client data packet -- Client side): ";
	display(recovered.crc);

	std::string message;
	Client_TCP client;
	client.create_socket();
	client.connect_socket("127.0.0.1", 12700);
	client.send_data_packet(my_packet);
	client.receive_message(message);
	std::cout << "Message Received: " << message << std::endl;
	

	int num;
	std::cin >> num;

	return 0;
}
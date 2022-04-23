#include <iostream>
#include <string>
#include "socket.h"

int main() {
	std::string message = "Hello";

	Client_TCP client;
	client.create_socket();
	client.connect_socket("127.0.0.1", 27000);
	client.send_data();

	return 0;
}


//#include <iostream>
//#include <string>
//#include "socket.h"
//
//int main() {
//	Server_TCP server("127.0.0.1", 27000);
//
//	server.create_socket();
//	server.bind_socket();
//	server.listen_for_connections();
//	server.accept_connection();
//	return 0;
//}
#include"modbusTCPServer.hpp"

int main(int argc, char** argv) {
	xtd::modbusTCPServer serv("127.0.0.1", 502);
	serv.setRegisterValue(5, 15);
	while (true) {
		serv.receive();
	}
	system("pause");
	return 0;
}
#include"modbusTCPServer.hpp"

int main(int argc, char** argv) {
	xtd::modbusTCPServer serv("10.9.24.65", 502);
	serv.setRegisterValue(5, 15);
	while (true) {
		serv.receive();
	}
	system("pause");
	return 0;
}
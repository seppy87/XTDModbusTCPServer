#include "modbusTCPServer.hpp"
#include<iostream>
#include"cast.hpp"

xtd::modbusTCPServer::modbusTCPServer(const std::string& host)
{
	auto val = xtd::splitHost(host, IPFamily::AUTOMATIC);
	if (std::get<2>(val))
		this->fam = IPFamily::IPV6;
	else
		this->fam = IPFamily::IPV4;
	this->init(std::get<0>(val), std::get<1>(val));
}

xtd::modbusTCPServer::modbusTCPServer(const std::string& ip, UINT16 port)
{
	
	if (ip.find(':')!=std::string::npos) {
		this->fam = IPFamily::IPV6;
	}
	else {
		this->fam = IPFamily::IPV4;
	}
	this->init(ip, port);
}

xtd::modbusTCPServer::~modbusTCPServer()
{
	WSACleanup();
}

void xtd::modbusTCPServer::init(const std::string& ip, UINT16 port)
{
	WSADATA wsdata;
	if (!WSAStartup(MAKEWORD(2, 0), &wsdata)) {
		//throw std::exception("COULD NOT STARTUP WINSOCKS");
	}
	try {
		if (this->fam == IPFamily::IPV4)
			this->s = socket(AF_INET, SOCK_STREAM, 0);
		else
			this->s = socket(AF_INET6, SOCK_STREAM, 0);
		if (this->s == INVALID_SOCKET)
			throw std::exception("INVALID SOCKET");
		//SOCKADDR_IN addr;
	}
	catch(std::exception ex){
		std::cout << ex.what() << '\n';
	}
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	memset(&this->master, 0, sizeof(SOCKADDR_IN));
	if (this->fam == IPFamily::IPV4)
		addr.sin_family = AF_INET;
	else
		addr.sin_family = AF_INET6;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	if(bind(this->s,(SOCKADDR*)&addr,sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		throw std::exception("SOCKET ERROR! BIND");
	listen(this->s, 10);
	int len = sizeof(this->master);
	this->connectedSocket = accept(this->s, (SOCKADDR*)& this->master,&len);

}

void xtd::modbusTCPServer::receive()
{
	if (this->header.data.size() != 0)
		this->header.data.clear();
	char buff[1024];
	memset(buff, 0, sizeof(char[1024]));

	auto r = recv(this->connectedSocket, buff, 1024, 0);
	if (r > 1023)
		throw std::exception("Size to big");
	buff[r] = '\0';
	this->preparePacket(buff, r);
	this->processOrder();
}

void xtd::modbusTCPServer::setupCallback(unsigned short functionCode, ModbusCallback callback)
{
	this->FunctionCallbacks.insert(std::make_pair(functionCode, callback));
}

void xtd::modbusTCPServer::reply()
{
	switch (this->header.FunctionCode) {
	case 3: {
		uint8_t transcationID[2];
		transcationID[0] = this->header.transactionID[0];
		transcationID[1] = this->header.transactionID[1];
		uint8_t protocolID[2];
		protocolID[0] = this->header.protocolID[0];
		protocolID[1] = this->header.protocolID[1];
		uint8_t length[2];

		uint8_t unitID = this->header.UnitIdentifier;
		uint8_t fc = this->header.FunctionCode;

		auto StartAddress = (xtd::uintCast<16>(this->header.data[0], this->header.data[1]).asValue<uint16_t>());
		auto size = (xtd::uintCast<16>(this->header.data[2], this->header.data[3]).asValue<uint16_t>());
		std::vector<unsigned char> prepared;
		for (uint16_t i = 0; i < size; i++) {
			auto val = static_cast<uint16_t>(this->data[StartAddress + i].to_ulong());
			auto ret = (xtd::uintCast<8>(val).asStdVector<uint8_t>());
			prepared.insert(prepared.end(), ret[1]);
			prepared.insert(prepared.end(), ret[0]);
		}
		uint16_t valsize = 2 + prepared.size();
		auto valsplit = (xtd::uintCast<8>(valsize).asStdVector<uint8_t>());
		length[0] = valsplit[1];
		length[1] = valsplit[0];
		std::vector<unsigned char> fullheader;
		//create header!
		fullheader.insert(fullheader.end(), transcationID[0]);
		fullheader.insert(fullheader.end(), transcationID[1]);
		fullheader.insert(fullheader.end(), protocolID[0]);
		fullheader.insert(fullheader.end(), protocolID[1]);
		fullheader.insert(fullheader.end(), length[0]);
		fullheader.insert(fullheader.end(), length[1]);
		fullheader.insert(fullheader.end(), unitID);
		fullheader.insert(fullheader.end(), fc);
		for (auto i : prepared) {
			fullheader.insert(fullheader.end(), i);
		}
		unsigned char* buf = &(fullheader[0]);
		send(this->connectedSocket, ((char*)buf), fullheader.size(), 0);
	}
		return;
	default:
		std::cout << "Not IMPLEMENTED OR WRONG FUNCTION CODE\n";
		return;
	}
}

void xtd::modbusTCPServer::setRegisterValue(int registerNo, unsigned int value, bool split)
{
	if (registerNo > this->data.size() - 1)
		throw std::exception("out of Register Boundries");
	if (split) {

	}
	this->data[registerNo] = value;
}

void xtd::modbusTCPServer::setCoilValue(int registerNo, unsigned int bit, bool value)
{
	if (registerNo >= this->data.size())
		throw std::exception("Chosen Register is beyond boundaries of the Memory!");
	this->data[registerNo].set(bit, value);
}

void xtd::modbusTCPServer::preparePacket(char buffer[], int size)
{
	//transaction ID
	this->header.transactionID[0] = buffer[0];
	this->header.transactionID[1] = buffer[1];
	//size= size -2;
	//Protocol Identifier
	this->header.protocolID[0] = buffer[2];
	this->header.protocolID[1] = buffer[3];
	//size -4
	//LENGTH
	this->header.Length[0] = buffer[4];
	this->header.Length[1] = buffer[5];
	//size-6
	//Unit Identifier
	this->header.UnitIdentifier = buffer[6];
	//size-7
	//Function Code
	this->header.FunctionCode = buffer[7];
	//size-8
	for (UINT16 i = 8; i < size; i++) {
		this->header.data.insert(this->header.data.end(), buffer[i]);
	}
}

void xtd::modbusTCPServer::processOrder()
{
	switch (this->header.FunctionCode)
	{
	case 1: // Read Coils;

		break;
	case 2: // Read Discrete Inputs
		break;
	case 3:
		if (this->FunctionCallbacks.count(3))
			this->FunctionCallbacks[3](this->header.data[1], this->header.data[3], std::nullopt);
		this->reply();
		break;
	case 4:	//Read Input Registers
		break;
	case 5:	//Write Single coil
		break;
	case 6:		//Write Single Register
		break;
	case 15:	//write multiple coils
		break;
	case 16:
		this->prepareReceivedValues();
		if (this->FunctionCallbacks.count(16))
			this->FunctionCallbacks[16](this->header.data[1], this->header.data[3], this->header.data);
		break;

	}
}

void xtd::modbusTCPServer::prepareReceivedValues()
{
	unsigned short StartAddress = static_cast<unsigned short>(this->header.data[1]);
	unsigned short size = static_cast<unsigned short>(this->header.data[3]);
	unsigned int index = 1;
	for (unsigned short i = 5; i < (size * 2) + 5; i = i + 2) {
		uint8_t high = this->header.data[i];
		uint8_t low = this->header.data[i + 1];
		auto fused = (xtd::uintCast<16>(high, low).asValue<uint16_t>());
		this->data[StartAddress + index] = fused;
		++index;
	}
	xtd::printStdArray<16>(this->data);
}

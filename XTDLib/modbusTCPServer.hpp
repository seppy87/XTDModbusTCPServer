#pragma once
#include"config.hpp"
#include"utilities.hpp"
#include<string>
#include<array>
#include<vector>
#include<functional>
#include<map>
#include<optional>
#include<bitset>

namespace xtd {

	using ModbusCallback = std::function<void(int address, int size, const std::optional<std::vector<unsigned char>>& data)>;
	using ModbusRegister = std::bitset<16>;
	using ModbusRegisters = std::array<ModbusRegister, 16>;

	struct ModbusRawHeader {
		std::array<unsigned char, 2> transactionID;
		std::array<unsigned char, 2> protocolID;
		std::array<unsigned char, 2> Length;
		unsigned char UnitIdentifier;
		unsigned char FunctionCode;
		std::vector<unsigned char> data;
	};

	

	class DLL modbusTCPServer
	{
	public:
		modbusTCPServer(const std::string& host);
		modbusTCPServer(const std::string& ip, UINT16 port);
		modbusTCPServer() = default;
		~modbusTCPServer();
		void init(const std::string& ip, UINT16 port);
		void receive();
		void setupCallback(unsigned short functionCode, ModbusCallback callback);
		void reply();
		void setRegisterValue(int registerNo, unsigned int value, bool split = false);
		void setCoilValue(int registerNo, unsigned int bit, bool value);

	protected:
		void preparePacket(char buffer[], int size);
		void processOrder();
		void prepareReceivedValues();
	private:
		SOCKET s;
		IPFamily fam;
		SOCKADDR_IN addr;
		SOCKADDR_IN master;
		SOCKET connectedSocket;
		ModbusRawHeader header;
		std::map<unsigned short, ModbusCallback> FunctionCallbacks;
		ModbusRegisters data;
	};
}

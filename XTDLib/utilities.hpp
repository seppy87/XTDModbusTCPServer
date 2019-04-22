#pragma once

#include"config.hpp"
#include<tuple>
#include<string>
#include<bitset>
#include<array>


namespace xtd {
	enum class IPFamily {
		IPV4 = 4,
		IPV6 = 6,
		AUTOMATIC = 0
	};

	std::tuple<std::string, UINT16,bool> splitHost(const std::string& host, IPFamily fam = IPFamily::IPV4);
	std::tuple<std::string, UINT16,bool> splitIPv4Host(const std::string& host);
	std::tuple<std::string, UINT16,bool> splitIPv6Host(const std::string& host);

	template<class T>
	void printArray(T arr[], size_t size) {
		for (UINT16 i = 0; i < size; i++) {
			std::cout << ((unsigned int)arr[i]) << '\n';
		}
	}

	template<size_t N>
	void printStdArray(const std::array<std::bitset<16>, N>& arr) {
		for (uint16_t i = 0; i < N; i++) {
			std::cout << arr[i].to_ulong() << '\n';
		}
	}
}
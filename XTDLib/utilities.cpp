#include "utilities.hpp"

std::tuple<std::string, UINT16,bool> xtd::splitHost(const std::string& host, IPFamily fam)
{
	if (fam == IPFamily::AUTOMATIC)
	{
		if (host[0] == '[')
			fam = IPFamily::IPV6;
		else
			fam = IPFamily::IPV4;
	}
	if (fam == xtd::IPFamily::IPV4)
		return xtd::splitIPv4Host(host);
	else
		return xtd::splitIPv6Host(host);
}

std::tuple<std::string, UINT16,bool> xtd::splitIPv4Host(const std::string& host)
{
	std::string ip = "";
	std::string port = "";
	bool doublePoint = false;
	for (auto c : host)
	{
		if (c == ':') {
			doublePoint = true;
			continue;
		}
		if (doublePoint)
			port += c;
		else
			ip += c;
	}
	return std::make_tuple(ip, std::stoi(port),false);
}

std::tuple<std::string, UINT16,bool> xtd::splitIPv6Host(const std::string& host)
{
	std::string ip = "";
	std::string port = "";
	bool isBracketEnd = false;
	for (auto c : host) {
		if (c == '[') continue;
		if (c == ']') {
			isBracketEnd = true;
			continue;
		}
		if (isBracketEnd) {
			if (c == ':') continue;
			port += c;
		}
		else {
			ip += c;
		}
	}
	return std::make_tuple(ip, std::stoi(ip),true);
}

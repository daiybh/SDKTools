#pragma  once
#include <string>
namespace HttpClient {

	struct ResponseItem
	{
		int state;//0:fail 1:success
		std::string errmsg;
		std::vector<std::string> ips;
		std::vector<std::string> texts;
		std::string token;
	};

	 ResponseItem isInDsBlack(std::string ip, std::string plateNumber);
};
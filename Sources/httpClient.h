#pragma  once
#include <string>
#include "../wu_camera-sdk/inc/logLib.h"
namespace HttpClient {

	struct ResponseItem
	{
		int state;//0:fail 1:success
		std::string errmsg;
		std::vector<std::string> ips;
		std::vector<std::string> texts;
		std::string token;
	};

	 ResponseItem isInDsBlack(simplyLogger _logger,std::string ip, std::string plateNumber);
};
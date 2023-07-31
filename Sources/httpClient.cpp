#include "httplib.h"

#include "httpClient.h"
#include "fmt/format.h"
#include "StringHelper.h"

#include "nlohmann/json.hpp"

namespace HttpClient {
	//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResponseItem, state, errmsg,ips,texts,token);

	inline void to_json(nlohmann::json& nlohmann_json_j, const ResponseItem& nlohmann_json_t) {
		nlohmann_json_j["state"] = nlohmann_json_t.state; nlohmann_json_j["errmsg"] = nlohmann_json_t.errmsg; nlohmann_json_j["ips"] = nlohmann_json_t.ips; nlohmann_json_j["texts"] = nlohmann_json_t.texts; nlohmann_json_j["token"] = nlohmann_json_t.token;
	} 
	inline void from_json(const nlohmann::json& nlohmann_json_j, ResponseItem& nlohmann_json_t) {

		nlohmann_json_j.at("state").get_to(nlohmann_json_t.state); 
		nlohmann_json_j.at("token").get_to(nlohmann_json_t.token);
		if (nlohmann_json_t.state == 0) {
			nlohmann_json_j.at("errmsg").get_to(nlohmann_json_t.errmsg);
			return;
		}
		nlohmann_json_j.at("ips").get_to(nlohmann_json_t.ips); 
		nlohmann_json_j.at("texts").get_to(nlohmann_json_t.texts); 
	}
}

std::string ansc_to_UTF8(std::string ascii)
{
	std::wstring unicode = ascii_to_unicode(ascii.data(), ascii.length());
	auto utf8 = unicode_to_utf8(unicode);

	return utf8;
}
HttpClient::ResponseItem HttpClient::isInDsBlack(simplyLogger _logger, std::string ip, std::string plateNumber)
{
	// HTTP
	httplib::Client cli("http://127.0.0.1:21983");
	//std::string newUrl = $"{url}?ip={ip}&plateNumber={plateNumber}&token={DateTime.Now.ToString("yyyyMMddHHmmssFFFF")}";

	//auto res = cli.Get("/isindsblack");
	httplib::Params params{
		{ "ip", ip },
		{ "plateNumber", ansc_to_UTF8(plateNumber) },
		{ "token", fmt::format("{}",GetTickCount()) }
	};
	auto aParams = httplib::detail::params_to_query_str(params);

	std::string getUrl = fmt::format("/isindsblack?{}", aParams);
	_logger->info("isInDsBlack query {}", getUrl);
	auto res = cli.Get(getUrl.c_str());

	/*
	state:
	msg:
	ips:
	text:
	*/
	ResponseItem item;
	if (res == nullptr)
	{
		_logger->info("isInDsBlack res == nullptr");
		item.state = 0;
		item.errmsg = "connect http://127.0.0.1:21983 failed.";
	}
	else if (res->status != 200) {
		_logger->info("isInDsBlack res->status({}) != 200",res->status);
		item.state = 0;
		item.errmsg = fmt::format("status={}", res->status);
	}
	else {
		_logger->info("isInDsBlack  200 body:\n{}", res->body);
		nlohmann::json j= nlohmann::json::parse(res->body);
		from_json(j, item);
	}
	return item;
}


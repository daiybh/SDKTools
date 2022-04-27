#pragma  once
#include "fmt/format.h"
#include <fmt/chrono.h>
static char hex_table[] = { '0','1','2','3','4' ,'5' ,'6' ,'7' ,'8' ,'9','A','B' };
#include "config.h"
class HandleCmd
{
public:
	HandleCmd() {
		m_sn = Config::instance().serverSN;
		makeCarcomingCmd("湘AC8888","192.168.0.123");

	}
	char bbcCheck(char* buffer, int nLen)
	{
		char xBBC = buffer[0];
		for (int i = 1; i < nLen; i++) {
			xBBC ^= buffer[i];
		}
		return xBBC;
	}
	void buildCmd() {
		std::string xCmd = "%CLOD0036CARDNUM 00\r\nBS20220001, 湘AC8888, 20220407163254\r\n5B@";
		
		
		parseCmd(xCmd.data(),xCmd.length());
	}
	std::string makeHeartCmd() {
		std::string cmd = "%CLOD0027ONLINE  00\r\n";
		std::string body = fmt::format("{},{}\r\n", m_sn, getTimeStr());
		return buildAllCmd(cmd, body);
	}
	
	std::string makeRaisePoleResponse(std::string &ip) {
	/*客户端响应的数据:
		% RLOD0027RISEPOLE00
			BS20220001, 192.168.0.243, 20220407191532
			2A@*/
		std::string cmd = "%RLOD0027RISEPOLE00\r\n";
		std::string body = fmt::format("{},{},{}\r\n", m_sn, ip, getTimeStr());

		return buildAllCmd(cmd,body);
	}
	std::string makeCarcomingCmd(std::string carNo,std::string ip) {
		/*
		%CLOD0036CARDNUM 00
BS20220001,湘AC1079,20220426170021
00@
*/
		std::string cmd = "%CLOD0036CARDNUM 00\r\n";
		std::string	body = fmt::format("{},{},{},{}\r\n",m_sn, carNo, ip, getTimeStr());
		return buildAllCmd(cmd, body);
	}
	bool parseCmd(char*pData,int nLen) {
		char bbcCode = bbcCheck(pData+1, nLen - 6);
		char x1 = pData[nLen - 4];
		char x2 = pData[nLen - 3];
		



		char c1 = hex_table[bbcCode >>4];
		char c2 = hex_table[bbcCode &0x0F];
		if (c1 != x1 && x2 != c2)return false;
		return true;
	}

private:
	std::string getTimeStr() {
		std::time_t t = std::time(nullptr);

		// Prints "The date is 2020-11-07." (with the current date):
		return fmt::format("{:%Y%m%d%H%M%S}", fmt::localtime(t));

	}
	std::string buildAllCmd(std::string header, std::string& body) {
		int nLen = body.length();
		header[5] = nLen / 1000 + '0';
		header[6] = (nLen % 1000) / 100 + '0';
		header[7] = (nLen % 100) / 10 + '0';
		header[8] = (nLen % 10) / 1 + '0';

		std::string allCmd = fmt::format("{}{}", header, body);
		char bbcCode = bbcCheck(allCmd.data() + 1, allCmd.length() - 3);


		char c1 = hex_table[bbcCode >> 4];
		char c2 = hex_table[bbcCode & 0x0F];
		std::string a = fmt::format("{}{}{}@", allCmd, c1, c2);

		return a;
	}
	std::string m_sn = "BS20220001";
	

};
#pragma  once
#include <string>


struct  Header {
	char head;//'%'
	char cmdFlag;// C R
	char cmdType[3];
	char dataLen[4];
	char text[8];
	char endCode[2];
	char end[2];// /r /n

	bool isValid() {
		return head == '%' && end[0]=='\r' && end[1] == '\n';
	}
	int getchar(char p) {
		if (p >= '0' && p <= '9')
			return p - '0';
		if (p >= 'a' && p<='f')
			return p - 'a';
		if (p >= 'A' && p <= 'F')
			return p - 'A';
		return 0;
	}
	int getDataLen() {
		return getchar(dataLen[0]) * 1000 + getchar(dataLen[1]) * 100 + getchar(dataLen[2]) * 10 + getchar(dataLen[3]);
	}
};
struct RisePole:public Header
{
	/*
	 %CLOD0027RISEPOLE00
 BS20220001,192.168.0.243,20220407163254
 25@

 客户端响应的数据:
 %RLOD0027RISEPOLE00
 BS20220001,192.168.0.243,20220407191532
 2A@
 */
	char body[1];
	bool isValid()
	{
		if (!Header::isValid())return false;
		return (stricmp(text, "RISEPOLE") == 0);
	}



};

class TCPBase
{
public:
	std::string ReadData(uint64_t clientSocket);
	int ReadData(uint64_t clientSocket, char* buffer, int nLen);
};
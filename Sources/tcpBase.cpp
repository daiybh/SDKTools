#include "tcpBase.h"
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>

std::string TCPBase::ReadData(uint64_t clientSocket)
{
	char buffer[1024] = { 0 };
	if (ReadData(clientSocket, buffer, 1024) > 0)
		return buffer;
	return "";
}

int TCPBase::ReadData(uint64_t clientSocket, char* buffer, int nLen)
{
	char* p = buffer;
	do {
		int size = recv(clientSocket, p, sizeof(Header), 0);
		if (size == SOCKET_ERROR)
			return -1;
		if (size != sizeof(Header))
		{
			break;
		}
		Header* pHeader = (Header*)p;
		if (!pHeader->isValid()) break;

		p += sizeof(Header);

		int nendLen = pHeader->getDataLen() + 3;
		size = recv(clientSocket, p, nendLen, 0);
		if (size == SOCKET_ERROR)
		{			
			return -1;
		}
		if (size != nendLen)
			break;
		p += nendLen-1;
		if (*p == '@' && *(p - 4) == '\r' && *(p - 3) == '\n')
		{
			p++;
			*p = '\0';
			return p-buffer;
		}
	} while (0);
	//check if is socket Error
	return 0;
}

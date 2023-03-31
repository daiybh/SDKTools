
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include "tcpClient.h"

TcpClient::TcpClient() {
	m_sockfd = 0;
}
TcpClient::~TcpClient() {
	close(m_sockfd);
}
bool TcpClient::ConnectToHost(const char* serverip, const int port) {
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct hostent* h;
	if ((h = gethostbyname(serverip)) == 0) {
		close(m_sockfd);
		m_sockfd = 0;
		return false;
	}
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
	if (connect(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		close(m_sockfd);
		m_sockfd = 0;
		return false;
	}
	return true;
}
int TcpClient::Send(const char* buffer, const int buflen) {
	return send(m_sockfd, buffer, buflen, 0);
}
int TcpClient::Recv(char* buffer, const int buflen) {
	return recv(m_sockfd, buffer, buflen, 0);
}

void TcpClient::close(int fd)
{
	closesocket(fd);
}

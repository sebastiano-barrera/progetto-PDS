// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <list>

#include "protocol.pb.h"
#include "global.h"


#pragma comment(lib, "Ws2_32.lib")



#define MAX_CONN 5


SOCKET sockInit(SHORT port);


std::condition_variable cv;
WindowsList windows_list;
ClientList pending;
ClientList active;

void checkWindowsEvents();
void threadPoolInit(int n);


int main(int argc, char**argv)
{
	std::string path(argv[0]);
	std::string delimiter("\\");
	std::string prog_name = path.substr(path.find_last_of("\\"), path.length());
	if (argc != 2) {
		std::cerr << "invalid arguments, try ." << prog_name << " <port#>" << std::endl;
		exit(1);
	}

	short port = atoi(argv[1]);
	SOCKET connected, s = sockInit(port);
	std::thread(checkWindowsEvents).detach();
	threadPoolInit(3);
	while (true) {
		struct sockaddr_in caddress;
		int length = sizeof(struct sockaddr_in);
		connected = accept(s, (struct sockaddr*)&caddress, &length);
		char address[64];
		std::string s((char*) inet_ntop(AF_INET, &caddress, address, 32));
		std::cout << "New connection established\nclient port: " << ntohs(caddress.sin_port) << " address: "<<s<<std::endl;
		pending.addClient(Client(connected));
	}
	closesocket(s);
	WSACleanup();

	return 0;
}

void serveClient() {
	while (true) {
		auto& client = active.addClient(pending.getClient());
		client.serve();
		active.cleanup();
	}
}

void checkWindowsEvents() {
	//std::cout << "Lanciato thread per controllare gli eventi" << std::endl;
	while (true) {
		//windows_list.printProcessList();
		windows_list.update();
		Sleep(250);
	}
}

void threadPoolInit(int n)
{
	for (int i = 0; i < n; i++) {
		std::thread(serveClient).detach();
	}
}

SOCKET sockInit(SHORT port) {
	WSADATA wsadata;
	struct sockaddr_in sin;
	SOCKET temp_sock;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		std::cerr << "WSAStartup() failed"<<std::endl;
		exit(1);
	}

	if ((temp_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		std::cerr << "socket() failed" << std::endl;
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if ((sin.sin_port = htons(port)) == 0) {
		std::cerr << "port assignment failed" << std::endl;
		exit(1);
	}

	if (bind(temp_sock, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
		std::cerr << "bind() failed" << std::endl;
		exit(1);
	}

	if (listen(temp_sock, MAX_CONN) == SOCKET_ERROR) {
		std::cerr << "listen() failed" << std::endl;
		WSACleanup();
		exit(1);
	}
	
	std::cout << "waiting for connections on port: " << port << " . . ." << std::endl;
	return temp_sock;
}
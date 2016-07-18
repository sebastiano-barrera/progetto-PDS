// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
//#include <Windows.h>
#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <thread>
#include "protocol.pb.h"

#define PORT_NO 25568
#define MAX_CONN 5

void checkWindowsEvents();
Windows_List w_list;


SOCKET sockInit();
int main()
{
	SOCKET connected, s = sockInit();
	std::thread(checkWindowsEvents).detach();
	while (true) {
		connected = accept(s, NULL, NULL);
		//pensavo alla creazione di un oggetto di tipo client che lancia un suo thread
		//in cui viene gestita la comunicazione col client
	}
	closesocket(s);
	WSACleanup();
	return 0;
}


void checkWindowsEvents() {
	std::cout << "Lanciato thread per controllare gli eventi" << std::endl;
	while (true) {
		w_list.printProcessList();
		w_list.Update();
		Sleep(250);

	}
}

SOCKET sockInit() {
	WSADATA wsadata;
	struct servent *pse;
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

	if ((sin.sin_port = htons(PORT_NO)) == 0) {
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
	
	std::cout << "waiting for connections..." << std::endl;
	return temp_sock;
}
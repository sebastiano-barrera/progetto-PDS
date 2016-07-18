// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//



#include "stdafx.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <Windows.h>

#include <iostream>
#include <thread>
#include <condition_variable>
#include <list>

#include "protocol.pb.h"
#include "global.h"
#define PORT_NO 25568
#define MAX_CONN 5


SOCKET sockInit();
std::condition_variable pendingClients;
Windows_List w_list;
ClientList pending;
ClientList active ;

void checkWindowsEvents();

int main()
{
	SOCKET connected, s = sockInit();
	std::thread(checkWindowsEvents).detach();
	while (true) {
		connected = accept(s, NULL, NULL);
		pending.addClient(Client(connected));
		pendingClients.notify_one();

		//pensavo alla creazione di un oggetto di tipo client che lancia un suo thread
		//in cui viene gestita la comunicazione col client

		//ALTERNATIVA
		//creo un pool di thread da usare per servire i client, in modo da non essere vulnerabili
		//ad un numero elevato di richieste. Quando si accetta una nuova connessione si aggiunge
		//ad una lista di client pendendi il nuovo client e si attende un thread libero per svegliarlo
	}
	closesocket(s);
	WSACleanup();
	return 0;
}



void serveClient() {
	while (true) {
		if (pending.getSize() > 0) {
			Client c = pending.getClient();
			active.addClient(c); //devo rimuoverlo dopo averlo servito
			c.serve();
		}
		else {
			//serve un lock qui ma sembra poco rappresentativo, ci sono da valutare soluzioni
			//alternative
			//pendingClients.wait();
		}
	}
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
// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <thread>
#include "protocol.pb.h"


void checkWindowsEvents();
Windows_List w_list;
int main()
{
	std::thread (checkWindowsEvents).detach();
	while (true);
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


// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <thread>
void checkWindowsEvents();
Windows_List list;
int main()
{
	std::thread t1(checkWindowsEvents);
	t1.detach();
	while (true);
	return 0;
}


void checkWindowsEvents() {
	std::cout << "Lanciato thread per controllare gli eventi" << std::endl;
	while (true) {
		list.printProcessList();
		list.Update();
		Sleep(2500);

	}
}


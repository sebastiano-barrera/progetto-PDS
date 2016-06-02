// pds_server.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "Windows_List.h"

int main()
{
	Windows_List list;
	list.Populate();
	list.printProcessList();
    return 0;
}


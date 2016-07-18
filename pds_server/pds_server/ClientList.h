#pragma once
#include "Client.h"
#include "global.h"
#include "Process_Window.h"
#include <list>
#include <mutex>

class ClientList
{
	//FIFO QUEUE
	std::list<Client> clients;
	std::mutex lock;
	unsigned int size;

public:
	ClientList();
	~ClientList();
	void addClient(Client c);
	void notify(Process_Window wnd, Process_Window::status s);
	unsigned int getSize();
	Client getClient();
};


#pragma once
#include <list>
#include <mutex>
#include "Client.h"
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
	Client getClient();
	unsigned int getSize();
};


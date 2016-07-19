#pragma once
#include "Client.h"
#include "Process_Window.h"
#include <list>
#include <mutex>

class ClientList
{
	//FIFO QUEUE
	std::condition_variable cv_;
	std::list<Client> clients;
	mutable std::mutex lock_;
	unsigned int size_;

public:
	ClientList();
	Client& addClient(Client c);
	void notify(ProcessWindow wnd, ProcessWindow::Status s);
	unsigned int size() const;
	Client getClient();
	void cleanup();
};


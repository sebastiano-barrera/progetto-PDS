#include "stdafx.h"
#include "ClientList.h"


ClientList::ClientList() : size(0)
{
}


ClientList::~ClientList()
{
}

void ClientList::addClient(Client c)
{
	std::lock_guard<std::mutex> lg(lock);
	clients.push_back(c);
	size++;
}

void ClientList::notify(Process_Window wnd, Process_Window::status s)
{
	std::lock_guard<std::mutex> lg(lock);
	for (auto c : clients) {
		c.sendMessage(wnd, s);
	}
}


Client ClientList::getClient()
{
	std::lock_guard<std::mutex> lg(lock);
	Client c = clients.front();
	clients.pop_front();
	size--;
	return c;
}

unsigned int ClientList::getSize()
{
	std::lock_guard<std::mutex> lg(lock);
	return this->size;
}

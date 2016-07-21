#include "stdafx.h"
#include "ClientList.h"
#include "global.h"

ClientList::ClientList() : size_(0) { }

Client& ClientList::addClient(Client c)
{
	std::lock_guard<std::mutex> lg(lock_);
	clients.push_back(std::move(c));
	auto& last_ref = clients.back();
	size_++;
	cv_.notify_all();
	return last_ref;
}

void ClientList::notify(ProcessWindow wnd, ProcessWindow::Status s)
{
	std::lock_guard<std::mutex> lg(lock_);
	for (auto &c : clients) {
		c.sendMessage(wnd, s);
	}
}

Client ClientList::getClient()
{
	std::unique_lock<std::mutex> ul(lock_);
	cv_.wait(ul, [this]() { return size_ > 0; });
	Client c = std::move(clients.front());
	clients.pop_front();
	size_--;
	return c;
}

unsigned int ClientList::size() const
{
	std::lock_guard<std::mutex> lg(lock_);
	return this->size_;
}

void ClientList::cleanup()
{
	std::lock_guard<std::mutex> lg(lock_);
	clients.remove_if([](const Client &c) {return c.isClosed(); }); //rimuovo i client non più attivi
	size_ = clients.size();
}

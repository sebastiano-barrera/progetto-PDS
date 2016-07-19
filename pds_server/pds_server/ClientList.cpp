#include "stdafx.h"
#include "ClientList.h"
#include "global.h"

ClientList::ClientList() : size_(0) { }

Client& ClientList::addClient(Client c)
{
	std::lock_guard<std::mutex> lg(lock_);
	clients.push_back(c);
	auto& last_ref = clients.back();
	size_++;
	cv_.notify_all();
	return last_ref;
}

void ClientList::notify(ProcessWindow wnd, ProcessWindow::Status s)
{
	std::lock_guard<std::mutex> lg(lock_);
	for (auto c : clients) {
		c.sendMessage(wnd, s);
	}
}

Client ClientList::getClient()
{
	std::unique_lock<std::mutex> lg(lock_);
	cv_.wait(lg, [this]() { return size_ > 0; });
	Client c = clients.front();
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
	/// TODO
	/// STUB
}

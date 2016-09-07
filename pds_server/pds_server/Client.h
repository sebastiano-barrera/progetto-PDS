#pragma once
#include "ProcessWindow.h"
#include "WindowsList.h"

class Client
{
	SOCKET sck;
	bool isClosed_;
	std::mutex cLock_;

public:
	~Client();
	Client(SOCKET s);
	void serve();
	void sendMessage(ProcessWindow wnd, ProcessWindow::Status s);
	friend void swap(Client &c1, Client &c2);
	bool isClosed() const;
	Client(Client&& src);

private:
	bool sendProcessList();
	void readMessage();
	void closeConnection();
	Client(const Client& src);

};


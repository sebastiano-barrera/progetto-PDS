#pragma once
#include "ProcessWindow.h"
#include "WindowsList.h"

class Client
{
	SOCKET sck;
	bool isClosed_;

public:
	~Client();
	Client(SOCKET s);
	void serve();
	void sendMessage(ProcessWindow wnd, ProcessWindow::Status s);

	inline bool isClosed() const {
		throw std::logic_error("not yet implemented");
	}

private:
	bool sendProcessList();
	void readMessage();
	void closeConnection();

};


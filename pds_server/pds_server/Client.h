#pragma once
#include "Process_Window.h"
#include "WindowsList.h"

class Client
{
	SOCKET sck;

public:
	//enum Status {W_CLOSED, W_OPENED, W_ONFOCUS};
	Client();
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

};


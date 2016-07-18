#pragma once
#include "Process_Window.h"
#include "Windows_List.h"
#include "global.h"

class Client
{
	SOCKET sck;

public:
	//enum status {W_CLOSED, W_OPENED, W_ONFOCUS};
	Client();
	~Client();
	Client(SOCKET s);
	void serve();
	void sendMessage(Process_Window wnd, Process_Window::status s);


private:
	bool sendProcessList();
	void readMessage();

};


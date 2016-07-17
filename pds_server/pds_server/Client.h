#pragma once
#include "Process_Window.h"
#include "Windows_List.h"
class Client
{
	SOCKET sck;
public:
	enum status {W_CLOSED, W_OPENED, W_ONFOCUS};
	Client();
	~Client();
	Client(Windows_List list);
	void readMessage();
	void sendMessage(Process_Window wnd,status s);
};


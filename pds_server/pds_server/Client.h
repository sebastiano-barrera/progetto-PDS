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
	Client(SOCKET s);
	void serve(Windows_List list);
private:
	bool sendProcessList(Windows_List list);
	void readMessage(Windows_List list);
	void sendMessage(Process_Window wnd, status s);

};


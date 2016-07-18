#include "stdafx.h"
#include "Client.h"
#include "protocol.pb.h"
#include <iostream>
#include <string>
#include <cstring>
#include <atlbase.h>
#include <atlconv.h>
#define MAXWAIT 120

Client::Client()
{

}


Client::~Client()
{
}

Client::Client(SOCKET s)
{
	this->sck = s;
}

bool Client::sendProcessList(Windows_List list)
{
	uint32_t size=0;
	msgs::AppList msg;
	std::string s_msg;
	std::list<Process_Window> l = list.WindowsList();
	for (auto it = l.begin(); it != l.end(); it++) {
		msgs::Application* app=msg.add_apps();
		app->set_id(it->GetId());
		app->set_name(it->GetTitle());
	}
	size = msg.ByteSize();
	s_msg = msg.SerializeAsString();
	std::cout <<"serialized msg size:"<<size<<std::endl<< s_msg<< std::endl;
	size = htonl(size);
	//DA GESTIRE IL CASO IN CUI RIESCA L'INVIO DELLA DIMENSIONE MA NON DELLA LISTA
	if (send(sck, (char*)&size, sizeof(u_long), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending message size" << std::endl;
		return false;
	}

	if (send(sck, (char*)&s_msg, s_msg.size(), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
		return false;
	}
	return true;
}

void Client::readMessage()
{
	uint32_t size=0;
	msgs::KeystrokeRequest msg;
	std::string buffer;
	fd_set readset;
	struct timeval tv;
	int res;
	FD_ZERO(&readset);
	FD_SET(this->sck, &readset);
	while (true) {
		tv.tv_sec = MAXWAIT;
		tv.tv_usec = 0;
		res = select(0, &readset, NULL, NULL, &tv);
		if (res > 0) {
			if (size == 0) {
				res = recv(sck, (char*)&size, sizeof(uint32_t), MSG_OOB);
			}
			else {
				res = recv(sck, (char*)&buffer, ntohl(size), MSG_OOB);
				size = 0;
			}
			if (res == 0) {
				//CONNECTION CLOSED BY CLIENT
				closesocket(sck);
				break;
			}
			else {
				if (!msg.ParseFromString(buffer)) {
					std::cerr << "an error occurred while parsing the msg" << std::endl;
				}
				else {
					//devo recuperare la finestra e mandare la combinazione di tasti
				}
			}
		}
		else if (res == 0) {
			closesocket(sck);
			break;
		}
		else { //SOCKET ERROR
			closesocket(sck);
			break;
		}
	}
	/*
		if (recv(sck, (char*)&size, sizeof(uint32_t), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an error occurred while reading msg size" << std::endl;
	}
	if (recv(sck, (char*)&buffer, ntohl(size), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an error occurred while reading msg" << std::endl;
	}

	if (!msg.ParseFromString(buffer)) {
		std::cerr << "an error occurred while parsing the msg" << std::endl;
	}
	
	*/
}

void Client::sendMessage(Process_Window wnd, status s)
{
	msgs::Application opened;
	msgs::AppDestroyed closed;
	msgs::AppGotFocus focus;
	msgs::Event event;
	std::string msg;
	uint32_t size;
	
	switch (s) {
	case W_OPENED:
		opened.set_name(wnd.GetTitle());
		opened.set_id(wnd.GetId());
		event.set_allocated_created(&opened);
		break;
	case W_CLOSED:
		closed.set_id(wnd.GetId());
		event.set_allocated_destroyed(&closed);
		break;
	case W_ONFOCUS:
		focus.set_id(wnd.GetId());
		event.set_allocated_got_focus(&focus);
		break;
	}
	size = event.ByteSize();
	msg = event.SerializeAsString();
	std::cout << "Serialized msg size: " << size << std::endl << msg << std::endl;
	
	size = htonl(size);
	if (send(sck, (char*)&size, sizeof(u_long), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending message size" << std::endl;
	}

	if (send(sck, (char*)&msg, msg.size(), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
	}

}

void Client::serve(Windows_List lst)
{
	this->sendProcessList(lst);
	this->readMessage();
}

#include "stdafx.h"
#include "Client.h"
#include "global.h"
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

void Client::serve()
{
	this->sendProcessList();
	this->readMessage();
}

bool Client::sendProcessList()
{
	uint32_t size_=0;
	msgs::AppList msg;
	std::string s_msg;

	auto windows = windows_list.windows();
	for (auto it = windows.begin(); it != windows.end(); it++) {
		msgs::Application* app = msg.add_apps();
		app->set_id((uint64_t) it->handle());
		app->set_name(it->title());
	}
	
	size_ = msg.ByteSize();
	s_msg = msg.SerializeAsString();
	std::cout <<"serialized msg size:"<<size_<<std::endl<< s_msg<< std::endl;
	size_ = htonl(size_);
	//DA GESTIRE IL CASO IN CUI RIESCA L'INVIO DELLA DIMENSIONE MA NON DELLA LISTA
	if (send(sck, (char*)&size_, sizeof(u_long), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending message size" << std::endl;
		return false;
	}

	if (send(sck, (char*)&s_msg, s_msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
		return false;
	}
	return true;
}

void Client::readMessage()
{
	uint32_t size_=0;
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
			if (size_ == 0) {
				res = recv(sck, (char*)&size_, sizeof(uint32_t), 0);
			}
			else {
				res = recv(sck, (char*)&buffer, ntohl(size_), 0);
				size_ = 0;
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
					ProcessWindow((HWND) msg.app_id()).sendKeystroke(msg);
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
		if (recv(sck, (char*)&size_, sizeof(uint32_t), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an error occurred while reading msg size_" << std::endl;
	}
	if (recv(sck, (char*)&buffer, ntohl(size_), MSG_OOB) == SOCKET_ERROR) {
		std::cerr << "an error occurred while reading msg" << std::endl;
	}

	if (!msg.ParseFromString(buffer)) {
		std::cerr << "an error occurred while parsing the msg" << std::endl;
	}
	
	*/
}

void Client::sendMessage(ProcessWindow wnd, ProcessWindow::Status s)
{
	msgs::Application opened;
	msgs::AppDestroyed closed;
	msgs::AppGotFocus focus;
	msgs::Event event;
	std::string msg;
	uint32_t size_;
	
	std::unique_ptr<msgs::Icon> icon_ptr;

	switch (s) {
	case ProcessWindow::W_OPENED:
		opened.set_name(wnd.title());
		opened.set_id((uint64_t) wnd.handle());
		icon_ptr = wnd.encodeIcon();
		opened.set_allocated_icon(icon_ptr.get());
		event.set_allocated_created(&opened);
		break;
	case ProcessWindow::W_CLOSED:
		closed.set_id((uint64_t) wnd.handle());
		event.set_allocated_destroyed(&closed);
		break;
	case ProcessWindow::W_ONFOCUS:
		focus.set_id((uint64_t) wnd.handle());
		event.set_allocated_got_focus(&focus);
		break;
	}
	size_ = event.ByteSize();
	msg = event.SerializeAsString();
	std::cout << "Serialized msg size: " << size_ << std::endl << msg << std::endl;
	
	size_ = htonl(size_);
	if (send(sck, (char*)&size_, sizeof(u_long), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending message size" << std::endl;
	}

	if (send(sck, (char*)&msg, msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
	}
}


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
#define BUFFSIZE 1024

bool readN(SOCKET s, int size, char* buffer);
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
	std::cout << "--sending process list--" << std::endl;
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
	//std::cout <<"serialized msg size:"<<size_<<std::endl<< s_msg<< std::endl;
	size_ = htonl(size_);
	//DA GESTIRE IL CASO IN CUI RIESCA L'INVIO DELLA DIMENSIONE MA NON DELLA LISTA
	if (send(sck, (char*)&size_, sizeof(uint32_t), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending message size" << std::endl;
		return false;
	}

	if (send(sck, s_msg.c_str(), s_msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
		return false;
	}
	return true;
}

void Client::readMessage()
{
	std::cout << "--reading message--" << std::endl;
	uint32_t size_=0;
	msgs::KeystrokeRequest msg;
	if (readN(sck, 4, (char*)&size_)){
		std::cout << "---read size---" << std::endl;
		size_ = ntohl(size_);
		std::unique_ptr<char[]> buffer(new char[size_]);
		if (readN(sck, size_, buffer.get())) {
			std::wcout << "----read message----" << std::endl;
			msg.ParseFromArray(buffer.get(), size_);
		}
	}
}

void Client::sendMessage(ProcessWindow wnd, ProcessWindow::Status s)
{
	std::cout << "--sending message--" << std::endl;
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

	if (send(sck, msg.c_str(), msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an errror occurred while sending data" << std::endl;
	}
}

//reads exactly size byte
bool readN(SOCKET s, int size, char* buffer){
	fd_set readset;
	struct timeval tv;
	int left,res, received;
	FD_ZERO(&readset);
	FD_SET(s, &readset);
	left = size;
	while (left > 0) {
		tv.tv_sec = MAXWAIT;
		tv.tv_usec = 0;
		res = select(0, &readset, NULL, NULL, &tv);
		if (res > 0) {
			res = recv(s, buffer, left, 0);
			if (res == 0) {//connection closed by client
				return false;
			}

			left -= res;
			buffer += res;
		}
		else if (res == 0) { //timer expired
			return false;
		}
		else { //socket error
			return false;
		}
	}
	return true;
}

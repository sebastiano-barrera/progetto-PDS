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

Client::~Client()
{
	if (sck != INVALID_SOCKET) {
		closeConnection();
	}
}

Client::Client(SOCKET s):isClosed_(false)
{
	this->sck = s;
}

void Client::serve()
{
	if (this->sendProcessList()) {
		this->readMessage();
	}
	closeConnection();
}

bool Client::isClosed() const
{
	return isClosed_;
}

void swap(Client &c1, Client &c2) {
	std::swap(c1.sck, c2.sck);
	std::swap(c1.isClosed_, c2.isClosed_);
}

Client::Client(Client && src)
{
	sck = INVALID_SOCKET;
	isClosed_ = true;
	swap(*this, src);
}

bool Client::sendProcessList()
{
	std::cout << "-sending process list" << std::endl;
	uint32_t size_=0;
	msgs::AppGotFocus focus;
	msgs::AppList msg;
	std::string s_msg;

	focus.set_id((uint64_t)windows_list.onFocus()); //saving current onfocus window
	auto windows = windows_list.windows();
	for (auto it = windows.begin(); it != windows.end(); it++) {
		msgs::Application* app = msg.add_apps();
		app->set_id((uint64_t) it->handle());
		app->set_name(it->title());
	}
	
	size_ = msg.ByteSize();
	size_ = htonl(size_);
	s_msg = msg.SerializeAsString();

	//DA GESTIRE IL CASO IN CUI RIESCA L'INVIO DELLA DIMENSIONE MA NON DELLA LISTA
	if (send(sck, (char*)&size_, sizeof(uint32_t), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending message size" << std::endl;
		return false;
	}

	if (send(sck, s_msg.c_str(), s_msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending data" << std::endl;
		return false;
	}


	//sending on focus window
	size_ = focus.ByteSize();
	size_ = htonl(size_);
	s_msg = focus.SerializeAsString();


	if (send(sck, (char*)&size_, sizeof(uint32_t), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending onfocus message size" << std::endl;
		return false;
	}

	if (send(sck, s_msg.c_str(), s_msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending onfocus data" << std::endl;
		return false;
	}

	return true;
}

void Client::readMessage()
{
	std::cout << "--reading message" << std::endl;
	uint32_t size_=0;
	msgs::KeystrokeRequest msg;
	msgs::Event response;
	msgs::Response *rsp = new msgs::Response();
	std::string serialized_response;
	uint32_t size;

	if (readN(sck, 4, (char*)&size_)){
		std::cout << "---read size" << std::endl;
		size_ = ntohl(size_);
		std::cout << "keystroke size = " << size_ << std::endl;
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size_);
		if (readN(sck, size_, buffer.get())) {
			std::cout << "----read message" << std::endl;
			std::cout <<"\t"<< buffer.get()<< std::endl;
			msg.ParseFromArray(buffer.get(), size_);
			//controllo che la finestra richiesta sia quella onfocus, 
			//in caso positivo mando l'input
			HWND target = (HWND)msg.app_id();
			if (target == windows_list.onFocus()) {
				ProcessWindow(target).sendKeystroke(msg);
				rsp->set_req_id(msg.req_id());
				rsp->set_status(msgs::Response::Status::Response_Status_Success);
			}
			else {
				//send error msg
				rsp->set_req_id(msg.req_id());
				rsp->set_status(msgs::Response::Status::Response_Status_WindowLostFocus);
			}
			response.set_allocated_response(rsp);
			size = response.ByteSize();
			size = htonl(size);
			serialized_response = response.SerializeAsString();

			if (send(sck, (char*)&size, sizeof(u_long), 0) == SOCKET_ERROR) {
				std::cerr << "an error occurred while sending response size" << std::endl;
			}

			if (send(sck, serialized_response.c_str(), serialized_response.size(), 0) == SOCKET_ERROR) {
				std::cerr << "an error occurred while response data" << std::endl;
			}
		}
	}
}

void Client::closeConnection()
{
	std::cout << "----CLOSING CONNECTION" << std::endl;
	closesocket(sck);
	sck = INVALID_SOCKET;
	isClosed_ = true;
}

void Client::sendMessage(ProcessWindow wnd, ProcessWindow::Status s)
{
	std::cout << "--sending message" << std::endl;
	msgs::Application opened;
	msgs::AppDestroyed closed;
	msgs::AppGotFocus focus;
	msgs::Event event;
	std::string msg;
	uint32_t size;
	
	// Note: the `set_allocated_*` methods take ownership of the passed pointer,
	// and will `delete` them at the exit of the scope.
	switch (s) {
		case ProcessWindow::W_OPENED: {
			auto opened = new msgs::Application();

			opened->set_name(wnd.title());
			opened->set_id((uint64_t)wnd.handle());
			auto icon_uniq_ptr = wnd.encodeIcon();
			opened->set_allocated_icon(icon_uniq_ptr.release());
			event.set_allocated_created(opened);
			break; 
		}
		case ProcessWindow::W_CLOSED: {
			auto closed = new msgs::AppDestroyed();

			closed->set_id((uint64_t)wnd.handle());
			event.set_allocated_destroyed(closed);
			break;
		}
		case ProcessWindow::W_ONFOCUS: {
			auto focus = new msgs::AppGotFocus();
			focus->set_id((uint64_t)wnd.handle());
			event.set_allocated_got_focus(focus);
			break;
		}
	}

	size = event.ByteSize();
	msg = event.SerializeAsString();
	std::cout << "Serialized msg size: " << size << std::endl << msg << std::endl;
	
	size = htonl(size);
	if (send(sck, (char*)&size, sizeof(u_long), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending message size" << std::endl;
	}

	if (send(sck, msg.c_str(), msg.size(), 0) == SOCKET_ERROR) {
		std::cerr << "an error occurred while sending data" << std::endl;
	}
}

//reads exactly size byte
bool readN(SOCKET s, int size, char* buffer){
	fd_set readset;
	struct timeval tv;
	int left, res;
	left = size;
	std::cout << "-----called readN to read " << size << " byte" << std::endl;
	memset(buffer, 0, size);
	while (left > 0) {
		FD_ZERO(&readset);
		FD_SET(s, &readset);
		tv.tv_sec = MAXWAIT;
		tv.tv_usec = 0;
		res = select(0, &readset, NULL, NULL, &tv);
		if (res > 0) {
			res = recv(s, buffer, left, 0);
			if (res == 0) {//connection closed by client
				return false;
			}
			else if (res == -1) { //recv() failed;
				return false;
			}
			left -= res;
			std::cout << "\treceived " << res << " left " << left << std::endl;
			if (left != 0) {
				buffer += res;
			}
			
		}
		else if (res == 0) { //timer expired
			return false;
		}
		else { //socket error
			return false;
		}
	}
	std::cout << "\t" << buffer << std::endl;
	return true;
}

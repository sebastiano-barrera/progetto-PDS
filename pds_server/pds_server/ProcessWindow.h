#pragma once
#include <Windows.h>

#include <string>

#include "protocol.pb.h"
#include "keys.pb.h"

class ProcessWindow
{
	std::wstring title_;
	HWND window_;
	HANDLE process_;
	HICON icon_;
	std::wstring moduleFileName_;
	//TODO: icon representation

public:
	enum Status { W_CLOSED, W_OPENED, W_ONFOCUS };

	ProcessWindow(HWND hWnd);
	
	void windowInfo() const;
	HWND handle() const;
	//std::wstring title();
	std::string title() const;
	bool sendKeystroke(msgs::KeystrokeRequest req);
	std::unique_ptr<msgs::Icon> encodeIcon() const;
};


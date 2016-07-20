#pragma once
#include <Windows.h>

#include <string>

#include "protocol.pb.h"
#include "keys.pb.h"

class ProcessWindow
{
	HWND window_;
	HANDLE process_;
	HICON icon_;
	std::wstring title_;
	std::wstring moduleFileName_;
	//TODO: icon representation

public:
	enum Status { W_CLOSED, W_OPENED, W_ONFOCUS };

	ProcessWindow(HWND hWnd);

	std::string title() const;
	std::unique_ptr<msgs::Icon> encodeIcon() const;
	bool sendKeystroke(msgs::KeystrokeRequest req);
	void windowInfo() const;
	HWND handle() const;
};


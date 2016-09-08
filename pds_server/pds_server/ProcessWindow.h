#pragma once
#include <Windows.h>

#include <string>

#include "protocol.pb.h"
#include "keys.pb.h"

class ProcessWindow
{
	HANDLE process_;
	HICON icon_;
	HWND window_;
	std::wstring title_; //window title
	std::wstring moduleFileName_; //executable file name

public:
	enum Status { W_CLOSED, W_OPENED, W_ONFOCUS };

	ProcessWindow(HWND hWnd);

	std::string title() const;
	std::string ProcessWindow::moduleFileName() const;
	std::unique_ptr<msgs::Icon> encodeIcon() const;
	bool sendKeystroke(msgs::KeystrokeRequest req);
	void windowInfo() const;
	HWND handle() const;
};


#pragma once
#include <string>
#include <Windows.h>
class Process_Window
{
	std::wstring title;
	unsigned int id;
	DWORD tid;
	HICON icon;
	HWND window;
	//TODO: icon representation
public:
	Process_Window();
	Process_Window(HWND hWnd);
	~Process_Window();
	void WindowInfo();
	HWND GetHandle();
	DWORD GetId();
	//std::wstring GetTitle();
	std::string GetTitle();
	bool SendKeyStroke(msgs::KeystrokeRequest req);
};


#include "stdafx.h"
#include "Process_Window.h"
#include <iostream>
#include <string>
#include <codecvt>


INPUT PressKey(int key);

Process_Window::Process_Window()
{
}

Process_Window::Process_Window(HWND hWnd)
{
	LPTSTR title = new WCHAR[256];
	HICON hicon;
	PICONINFO icon_info=nullptr;
	this->window = hWnd;
	this->tid = GetWindowThreadProcessId(hWnd, NULL);
	if (GetWindowText(hWnd, title, 256)>0) {
		this->title = std::wstring(title);
	}
	else {
		this->title = std::wstring();
	}

	hicon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_GETICON, ICON_SMALL2, NULL)); //getting the handle to window icon
	if (hicon != NULL) {
		this->icon = hicon;
	}

	//CURRENTLY NOT WORKING
	/* 
		if (!GetIconInfo(hicon, icon_info)) {
		std::cout << "geticoninfo() failed\n" << std::endl;
	}
	*/

	//TODO: icon retrieval
	//this->WindowInfo();
	delete[] title;
}


Process_Window::~Process_Window()
{
}

void Process_Window::WindowInfo()
{
	std::wcout << "Tid: "<<this->tid << "--> Title:" << this->title <<" handle: "<<this->window<< std::endl;
}

HWND Process_Window::GetHandle()
{
	return this->window;
}

DWORD Process_Window::GetId()
{
	return this->tid;
}
std::string Process_Window::GetTitle()
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conversion;
	return conversion.to_bytes(this->title);
}

bool Process_Window::SendKeyStroke(msgs::KeystrokeRequest req)
{
	int i=0,n= 0;
	INPUT ip[5];
	if (req.ctrl()) {
		ip[i++] = PressKey(0);
	}
	if (req.alt()) {
		ip[i++] = PressKey(0);
	}
	if (req.shift()) {
		ip[i++] = PressKey(0);
	}
	if (req.meta()) {
		ip[i++] = PressKey(0);
	}
	ip[i++] = PressKey(0);
	n = i;
	BringWindowToTop(window);
	if (SendInput(n, ip, sizeof(INPUT))) {
		while (i > 0) {
			ip[--i].ki.dwFlags = KEYEVENTF_KEYUP;
		}
		if (SendInput(n, ip, sizeof(INPUT))) {
			return true;
		}
	}
	std::cerr << "unable to send input to the selected window" << std::endl;

}

INPUT PressKey(int key) {
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.time = 0;
	input.ki.wScan = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = key;
	return input;
}

/*
std::wstring Process_Window::GetTitle()
{
	return title;
}
*/



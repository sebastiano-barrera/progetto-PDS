#include "stdafx.h"
#include "Process_Window.h"
#include <iostream>

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

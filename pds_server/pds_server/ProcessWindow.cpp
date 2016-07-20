#include "stdafx.h"
#include "ProcessWindow.h"
#include "global.h"

#include <iostream>
#include <string>
#include <codecvt>

INPUT PressKey(int key);

ProcessWindow::ProcessWindow(HWND hWnd) :
	window_(hWnd)
{
	LPTSTR title = new WCHAR[256];
	HICON hicon;
	PICONINFO icon_info=nullptr;
	
	if (GetWindowText(hWnd, title, 256) > 0)
		title_ = std::wstring(title);

	// Getting the handle to window_ icon_
	hicon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_GETICON, ICON_SMALL2, NULL));
	if (hicon != NULL)
		icon_ = hicon;

	//CURRENTLY NOT WORKING
	/* 
		if (!GetIconInfo(hicon, icon_info)) {
		std::cout << "geticoninfo() failed\n" << std::endl;
	}
	*/

	//TODO: icon_ retrieval
	//this->windowInfo();
	delete[] title;
}


void ProcessWindow::windowInfo() const
{
	std::wcout << "Window: Title:" << title_ << " handle: " << window_ << std::endl;
}

HWND ProcessWindow::handle() const
{
	return window_;
}

std::string ProcessWindow::title() const
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conversion;
	return conversion.to_bytes(this->title_);
}

bool ProcessWindow::sendKeystroke(msgs::KeystrokeRequest req)
{
	int num_mods = 0;
	INPUT ip[5];
	
	if (req.ctrl()) {
		ip[num_mods++] = PressKey(VK_CONTROL);
	}
	if (req.alt()) {
		ip[num_mods++] = PressKey(0);
	}
	if (req.shift()) {
		ip[num_mods++] = PressKey(0);
	}
	if (req.meta()) {
		ip[num_mods++] = PressKey(0);
	}
	
	// TODO: convertire da tasto di protocollo
	ip[num_mods++] = PressKey(0);
	
	BringWindowToTop(window_);
	//oppure
	SetForegroundWindow(window_);
	if (SendInput(num_mods, ip, sizeof(INPUT))) {
		for (int i = 0; i < num_mods; i++) {
			ip[i].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		if (SendInput(num_mods, ip, sizeof(INPUT))) {
			return true;
		}
	}

	std::cerr << "unable to send input to the selected window" << std::endl;
	return false;
}

INPUT PressKey(int key) {
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = key;
	input.ki.time = 0;
	input.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC); // TROVATO SU STACKOVERFLOW, SEMBRA FUNZIONARE ANCHE CON 0
	input.ki.dwFlags = 0;
	input.ki.dwExtraInfo = 0;
	return input;
}

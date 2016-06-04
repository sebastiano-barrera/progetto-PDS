#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <Windows.h>

BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
BOOL IsAltTabWindow(HWND hwnd);

using namespace std;

Windows_List::Windows_List()
{
	//std::cout << "Creating Windows list" << std::endl;
	if (EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this))) {
		//std::cout << "Windows list created" << std::endl;
	}
	else {
		std::cout << "Windows_List() failed\n" << std::endl;
	}
}


Windows_List::~Windows_List()
{
}

void Windows_List::addProcessWindow(HWND hWnd, Process_Window wnd)
{
	std::unique_lock<std::shared_mutex> lck(this->lock);	
	this->list.insert(std::pair<HWND, Process_Window> (hWnd, wnd));
}

void Windows_List::printProcessList()
{	
	std::shared_lock<std::shared_mutex> lck(this->lock);
	
	for (auto pw : this->list) {
		pw.second.WindowInfo();
	}
	cout << endl;
}

void Windows_List::Update()
{

	Windows_List updated; //getting the new list of windows
	bool closed;
	bool opened;
	unique_lock<shared_mutex> lck(this->lock); //the update process must be atomic
	
	//looking for the old windows in the new list --> EVENT: WINDOW CLOSED
	for (auto old_w : this->list) {
		closed = true;
		for (auto new_w : updated.list) {
			if (old_w.first == new_w.first) { //comparing handles
				closed = false;
				break;
			}
		}
		if (closed) {
			cout << "FINESTRA CHIUSA ";
			old_w.second.WindowInfo();
			
			//SEND EVENT
		}
	}

	//looking for new windows in the old list --> EVENT: NEW WINDOW CREATED
	for (auto new_w : updated.list) {
		opened = true;
		for (auto old_w : this->list) {
			if (new_w.first == old_w.first) { //comparing handles
				opened = false;
				break;
			}
		}
		if (opened) {
			cout << "NUOVA FINESTRA ";
			new_w.second.WindowInfo();
			//SEND EVENT
		}
	}
	this->list = updated.list; //updating the list
}



BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam) {
	Windows_List *w = reinterpret_cast<Windows_List*>(lParam);
	if (IsAltTabWindow(hWnd)) {
		w->addProcessWindow(hWnd, Process_Window(hWnd));
	}
	w = nullptr;
	return true;
}

BOOL IsAltTabWindow(HWND hwnd)
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	if (!IsWindowVisible(hwnd))
		return FALSE;

	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
	while (hwndTry != hwndWalk)
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup(hwndWalk);
		if (IsWindowVisible(hwndTry))
			break;
	}
	if (hwndWalk != hwnd)
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	// Tool windows should not be displayed either, these do not appear in the
	// task bar.
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	return TRUE;
}
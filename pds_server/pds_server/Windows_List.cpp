#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <Windows.h>


BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
BOOL IsAltTabWindow(HWND hwnd);
Windows_List::Windows_List()
{
}


Windows_List::~Windows_List()
{
}

void Windows_List::Populate()
{
	std::unique_lock<std::shared_mutex > lck(this->lock);
	std::cout << "Populating Windows list" << std::endl;
	if (EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this))) {
		std::cout << "Population process terminated" << std::endl;
	}
	else {
		std::cout << "Windows_List::Populate() failed\n" << std::endl;
	}

}

void Windows_List::addProcessWindow(Process_Window wnd)
{
	this->list.push_back(wnd);
}

void Windows_List::printProcessList()
{	
	std::shared_lock<std::shared_mutex> lck(this->lock);
	
	for (auto pw : this->list) {
		pw.WindowInfo();
	}
}



BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam) {
	Windows_List *w = reinterpret_cast<Windows_List*>(lParam);
	if (IsAltTabWindow(hWnd)) {
		w->addProcessWindow(Process_Window(hWnd));
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
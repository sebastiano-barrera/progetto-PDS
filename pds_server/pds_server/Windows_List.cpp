#include "stdafx.h"
#include "Windows_List.h"
#include <iostream>
#include <Windows.h>

uint32_t Windows_List::winId = 0;

BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
BOOL CALLBACK MyEnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
void MyEnumWindows(std::vector<HWND>* v);
BOOL IsAltTabWindow(HWND hwnd);

using namespace std;

void Windows_List::addProcessWindowNoLock(Process_Window wnd)
{
	this->list.insert(std::pair<uint32_t, Process_Window>(this->winId, wnd));
	do {
		this->winId++;

	} while (this->list.count(winId) != 0);
}

std::list<Process_Window> Windows_List::WindowsList()
{
	std::lock_guard<std::shared_mutex> lg(lock);
	std::list<Process_Window> windows;
	for (auto it = list.begin(); it != list.end(); it++) {
		windows.push_back(it->second);
	}
	return windows;
}

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

void Windows_List::addProcessWindow(Process_Window wnd)
{
	//cout << "waiting for lock";
	std::lock_guard<::shared_mutex> lck(this->lock);
	//cout << "Inserting ID: " << this->winId << " " ;
	//wnd.WindowInfo();
	//cout << "lock acquired";
	this->list.insert(std::pair<uint32_t, Process_Window>(this->winId, wnd));
	do {
		this->winId++;
		
	} while (this->list.count(winId) != 0);

}

void Windows_List::printProcessList()
{	
	std::shared_lock<std::shared_mutex> lck(this->lock);
	cout << "list size : " << this->list.size()<<endl;
	for (auto pw : this->list) {
		cout << "ID: " << pw.first << " ";
		pw.second.WindowInfo();
	}
	cout << endl;
}

void Windows_List::Update()
{

	//MANCA L'EVENTO DEL CAMBIO DI FUOCO

	GetForegroundWindow(); //handle alla finestra in primo piano

	bool closed;
	bool opened;
	std::vector<HWND> updated;
	std::lock_guard<shared_mutex> lck(this->lock); //the update process must be atomic
	std::vector<std::pair<uint32_t, Process_Window>> still_active; //keeps track of the windows still active
	std::vector<Process_Window> new_windows; //keeps track of the new window
	
	MyEnumWindows(&updated);
	//looking for the old windows in the new list --> EVENT: WINDOW CLOSED
	for (auto old : this->list) {
		closed = true;
		for (auto nw : updated) {
			if (old.second.GetHandle() == nw) { //comparing windows HANDLES
				closed = false;
				still_active.push_back(old);
				break;
			}
		}
		if (closed) {
			cout << "Closed  window :";
			old.second.WindowInfo();
			//SEND EVENT
			active.notify(old.second, Process_Window::W_CLOSED);

		}
	}

	//looking for new windows in the old list --> EVENT: NEW WINDOW CREATED
	for (auto nw : updated) {
		opened = true;
		for (auto old : this->list) {
			if (nw == old.second.GetHandle()) { //comparing windows HANDLES
				opened = false;
				break;
			}
		}
		if (opened) {
			Process_Window w(nw);
			cout << "Opened window : ";
			w.WindowInfo();
			new_windows.push_back(w);
			//SEND EVENT
			active.notify(w, Process_Window::W_OPENED);
		}
	}
	this->list.clear();
	this->list.insert(still_active.begin(), still_active.end());

	for (auto i : new_windows) {
		this->addProcessWindowNoLock(i); // we already owe a lock
	}
}

Process_Window Windows_List::getWindow(uint32_t id)
{
	return list[id];
}



BOOL CALLBACK EnumWindowsProc(__in HWND hWnd, __in LPARAM lParam) {
	Windows_List *w = reinterpret_cast<Windows_List*>(lParam);
	if (IsAltTabWindow(hWnd)) {
		w->addProcessWindow(Process_Window(hWnd));
	}
	w = nullptr;
	return true;
}

BOOL CALLBACK MyEnumWindowsProc(__in HWND hWnd, __in LPARAM lParam) {
	std::vector<HWND>*w = reinterpret_cast<std::vector<HWND>*>(lParam);
	if (IsAltTabWindow(hWnd)) {
		w->push_back(hWnd);
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

void MyEnumWindows(std::vector<HWND>* v) {
	if (EnumWindows(MyEnumWindowsProc, reinterpret_cast<LPARAM>(v))) {
		//std::cout << "Windows list created" << std::endl;
	}
	else {
		std::cout << "Windows_List() failed\n" << std::endl;
	}
}
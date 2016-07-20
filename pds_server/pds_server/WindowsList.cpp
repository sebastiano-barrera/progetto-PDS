#include "stdafx.h"
#include "WindowsList.h"
#include "global.h"

#include <iostream>
#include <Windows.h>

BOOL CALLBACK MyEnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
void MyEnumWindows(std::set<HWND>* v);
BOOL IsAltTabWindow(HWND hwnd);

using namespace std;

WindowsList::WindowsList()
{
	onFocus_ = GetForegroundWindow();
	MyEnumWindows(&winHandles_);
}

std::vector<ProcessWindow> WindowsList::windows() const
{
	std::lock_guard<std::mutex> lg(lock_);
	std::vector<ProcessWindow> windows;
	
	for (HWND handle : winHandles_)
		windows.emplace_back(handle);
	
	return windows;
}

void WindowsList::printProcessList()
{	
	auto procWins = this->windows();
	std::lock_guard<std::mutex> lck(this->lock_);
	for (const auto& pw : procWins)
		pw.windowInfo();
}

void WindowsList::update()
{
	// Elenco delle finestre aggiornato
	std::set<HWND> updated;
	std::lock_guard<mutex> lck(lock_); // the update process must be atomic
	
	MyEnumWindows(&updated);
	HWND currentFocus = GetForegroundWindow(); //getting foreground window handle

	// looking for the old windows --> EVENT: WINDOW CLOSED
	std::vector<HWND> closed_wins;
	std::set_difference(winHandles_.begin(), winHandles_.end(), updated.begin(), updated.end(), std::inserter(closed_wins, closed_wins.begin()));
	for (auto old_handle : closed_wins) {
		auto pw = ProcessWindow(old_handle);
		cout << "Closed window :";
		pw.windowInfo();
		//SEND EVENT
		active.notify(std::move(pw), ProcessWindow::W_CLOSED);
	}

	std::vector<HWND> new_wins;
	std::set_difference(updated.begin(), updated.end(), winHandles_.begin(), winHandles_.end(), std::inserter(new_wins, new_wins.begin()));
	//looking for new windows --> EVENT: NEW WINDOW CREATED
	for (auto new_handle : new_wins) {
		auto pw = ProcessWindow(new_handle);
		std::cout << "Opened window : ";
		pw.windowInfo();
		//SEND EVENT
		active.notify(pw, ProcessWindow::W_OPENED);
	}
	/*
	//potrebbe capitare che la finestra in foreground non sia tra quelle recuperate da MyEnumWindows, in questo caso
	//l'evento del cambio di fuoco non viene notificato e si aspetta il controllo successivo
	if (onFocus_ != currentFocus && updated.find(currentFocus) != updated.end()) {
		onFocus_ = currentFocus;
		auto pw = ProcessWindow(onFocus_);
		std::cout << "Focus changed" << std::endl;
		pw.windowInfo();
		//SEND EVENT
		active.notify(pw, ProcessWindow::W_ONFOCUS);
	}
	*/
	std::swap(winHandles_, updated);
}

HWND WindowsList::onFocus()
{
	return onFocus_;
}

void MyEnumWindows(std::set<HWND> *win_handles) {
	if (EnumWindows(MyEnumWindowsProc, reinterpret_cast<LPARAM>(win_handles))) {
		//std::cout << "Windows windows_ created" << std::endl;
	}
	else {
		std::cout << "WindowsList() failed\n" << std::endl;
	}
}

BOOL CALLBACK MyEnumWindowsProc(__in HWND hWnd, __in LPARAM lParam) {
	auto win_handles = reinterpret_cast<std::set<HWND>*>(lParam);
	if (IsAltTabWindow(hWnd)) {
		win_handles->insert(hWnd);
	}
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

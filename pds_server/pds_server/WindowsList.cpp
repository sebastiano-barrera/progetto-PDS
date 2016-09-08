#include "stdafx.h"
#include "WindowsList.h"
#include "global.h"

#include <iostream>
#include <Windows.h>

BOOL CALLBACK MyEnumWindowsProc(__in HWND hWnd, __in LPARAM lParam);
void MyEnumWindows(std::set<HWND>* v);
BOOL hasGUI(HWND hwnd);

WindowsList::WindowsList()
{
	onFocus_ = GetForegroundWindow(); //saving the current on focus windows
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
	std::lock_guard<std::mutex> lck(this->lock_); //someone may call update() while printing the list
	for (const auto& pw : procWins)
		pw.windowInfo();
}

void WindowsList::update()
{
	// updated windows list
	std::set<HWND> updated;
	std::lock_guard<std::mutex> lck(lock_); // the update process must be atomic

	MyEnumWindows(&updated);
	HWND currentFocus = GetForegroundWindow(); //getting foreground window handle

	// looking for the old windows --> EVENT: WINDOW CLOSED
	// winHandles - updated = closed windows 
	std::vector<HWND> closed_wins;
	std::set_difference(winHandles_.begin(), winHandles_.end(), updated.begin(), updated.end(), std::inserter(closed_wins, closed_wins.begin()));
	for (auto old_handle : closed_wins) {
		auto pw = ProcessWindow(old_handle);
		std::cout << "Closed window "<<old_handle << std::endl;
		//SENDING THE EVENT
		active.notify(std::move(pw), ProcessWindow::W_CLOSED);
	}

	// looking for new windows --> EVENT: NEW WINDOW CREATED
	// updated - winHandles = new windows
	std::vector<HWND> new_wins;
	std::set_difference(updated.begin(), updated.end(), winHandles_.begin(), winHandles_.end(), std::inserter(new_wins, new_wins.begin()));
	for (auto new_handle : new_wins) {
		auto pw = ProcessWindow(new_handle);
		std::cout << "Opened window "<< new_handle << std::endl;
		//SENDING THE EVENT
		active.notify(pw, ProcessWindow::W_OPENED);
	}
	
	// if the current foreground window is not listed, the focusChange event is notified on the next check
	if (onFocus_ != currentFocus && currentFocus!=NULL) {
		std::cout << "Focus changed, old focus : "<< onFocus_ << " current focus : " <<currentFocus << std::endl;
		if (updated.find(currentFocus) != updated.end()) { // the current foreground window is listed
			onFocus_ = currentFocus;
		}
		else { // the current foreground windows is NOT listed
			onFocus_ = (HWND)MAXUINT64; // this is how we represent the lost of focus
		}
		
		auto pw = ProcessWindow(onFocus_);	
		//SENDING THE EVENT
		active.notify(pw, ProcessWindow::W_ONFOCUS);
		onFocus_ = currentFocus; // we need to restore the proper value for onFocus_
	}
	std::swap(winHandles_, updated); // swapping the old and the new list
}

HWND WindowsList::onFocus()
{
	std::lock_guard<std::mutex> lg(lock_);
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
	if (hasGUI(hWnd)) {
		win_handles->insert(hWnd);
	}
	return true;
}

BOOL hasGUI(HWND hwnd)
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	if (!IsWindowVisible(hwnd))
		return FALSE;

	WCHAR buff[256];
	if (!GetWindowText(hwnd, buff, 256)) //if there is not title is very likely that is not a visible window
		return FALSE;
	
	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER); //get the root window
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

#pragma once

#include <Windows.h>

#include <vector>
#include <mutex>
#include <set>

#include "Process_Window.h"

// A thread-safe collection of windows. 
class WindowsList
{
	std::set<HWND> winHandles_;
	mutable std::mutex lock_;

public:
	WindowsList();

	std::vector<ProcessWindow> windows() const;
	void printProcessList();
	void update();
};

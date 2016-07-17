#pragma once
#include "Process_Window.h"
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <Windows.h>
#include <atomic>
class Windows_List
{
	static uint32_t winId;
	std::unordered_map < uint32_t, Process_Window> list;
	std::shared_mutex lock;
	void addProcessWindowNoLock(Process_Window wnd);

public:
	Windows_List();
	~Windows_List();
	std::list<Process_Window> WindowsList();
	void addProcessWindow(Process_Window wnd);
	void printProcessList();
	void Update();
protected:
};


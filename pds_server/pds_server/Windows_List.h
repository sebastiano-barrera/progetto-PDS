#pragma once
#include "Process_Window.h"
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <Windows.h>
class Windows_List
{
	std::unordered_map < HWND, Process_Window> list;
	std::shared_mutex lock;
	

public:
	Windows_List();
	~Windows_List();
	void addProcessWindow(HWND hWnd, Process_Window wnd);
	void printProcessList();
	void Update();
protected:
};


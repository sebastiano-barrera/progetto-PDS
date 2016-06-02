#pragma once
#include "Process_Window.h"
#include <vector>
#include <mutex>
#include <shared_mutex>

class Windows_List
{
	std::vector<Process_Window> list;
	std::shared_mutex lock;

public:
	Windows_List();
	~Windows_List();
	void Populate();
	void addProcessWindow(Process_Window wnd);
	void printProcessList();
protected:
	
};


#pragma once

#include <Windows.h>
#include <string>
#include <cstdint>

class ApplicationLogic {
public:
	// Get the ID of the current primary monitor
	static uint32_t GetPrimaryMonitorID();

	// Set a monitor as the primary monitor
	static long SetAsPrimaryMonitor(uint32_t id);

	// Start a process and wait for it to finish
	static int StartProcessAndWait(const wchar_t* processPath);

	// Execute the full application workflow: change monitor, start process, restore monitor
	static int ExecuteWithMonitorSwitch(int targetMonitorID, const wchar_t* exePath, const wchar_t* workDir = nullptr);
};

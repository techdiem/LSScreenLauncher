#include "ApplicationLogic.h"
#include <iostream>
#include <vector>

uint32_t ApplicationLogic::GetPrimaryMonitorID() {
	DISPLAY_DEVICE device;
	device.cb = sizeof(DISPLAY_DEVICE);
	uint32_t primID;
	for (primID = 0; EnumDisplayDevices(nullptr, primID, &device, 0); primID++) {
		if (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && 
			device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
			break;
		}
	}
	return primID;
}

long ApplicationLogic::SetAsPrimaryMonitor(uint32_t id) {
	DISPLAY_DEVICE device;
	DEVMODE deviceMode;
	device.cb = sizeof(DISPLAY_DEVICE);

	if (!EnumDisplayDevices(nullptr, id, &device, 0)) {
		return -1;
	}

	deviceMode.dmSize = sizeof(DEVMODE);
	if (!EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) {
		return -1;
	}

	POINTL offset = { deviceMode.dmPosition.x, deviceMode.dmPosition.y };
	deviceMode.dmPosition.x = 0;
	deviceMode.dmPosition.y = 0;

	long setSettingsResult = ChangeDisplaySettingsEx(
		device.DeviceName,
		&deviceMode,
		nullptr,
		CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET,
		nullptr
	);

	if (setSettingsResult != DISP_CHANGE_SUCCESSFUL) {
		return setSettingsResult;
	}

	// Update remaining devices
	for (uint32_t otherId = 0; EnumDisplayDevices(nullptr, otherId, &device, 0); ++otherId) {
		if (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && otherId != id) {
			deviceMode.dmSize = sizeof(DEVMODE);
			if (EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) {
				deviceMode.dmPosition.x -= offset.x;
				deviceMode.dmPosition.y -= offset.y;

				setSettingsResult = ChangeDisplaySettingsEx(
					device.DeviceName,
					&deviceMode,
					nullptr,
					CDS_UPDATEREGISTRY | CDS_NORESET,
					nullptr
				);
				if (setSettingsResult != DISP_CHANGE_SUCCESSFUL) {
					return setSettingsResult;
				}
			}
		}
	}

	// Apply settings
	setSettingsResult = ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, 0, nullptr);
	return setSettingsResult;
}

int ApplicationLogic::StartProcessAndWait(const wchar_t* processPath, const wchar_t* arguments) {
	// Process information struct
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	// Start information struct
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	std::wstring commandLine = L"\"";
	commandLine += processPath;
	commandLine += L"\"";
	if (arguments && arguments[0] != L'\0') {
		commandLine += L" ";
		commandLine += arguments;
	}

	std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
	mutableCommandLine.push_back(L'\0');

	// Launch process
	int procReturnCode = CreateProcess(
		nullptr, // Anwendung über Commandline starten
		mutableCommandLine.data(),  // Befehlszeilenargumente
		nullptr,             // Prozess-Sicherheitsattribute
		nullptr,             // Thread-Sicherheitsattribute
		FALSE,               // Erben von Handles
		0,                   // Erstellungsflags
		nullptr,             // Umgebungsvariablen
		nullptr,             // Arbeitsverzeichnis
		&startupInfo,       // Startinformationen
		&processInfo         // Prozessinformationen
	);

	// Wait for process exiting
	WaitForSingleObject(processInfo.hProcess, INFINITE);

	// Free process handles
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	//Return code 0 on success, else get return code
	if (procReturnCode != 0) {
		procReturnCode = GetLastError();
	}
	return procReturnCode;
}


int ApplicationLogic::ExecuteWithMonitorSwitch(int targetMonitorID, const wchar_t* exePath, const wchar_t* arguments) {
	// Get currently active primary monitor
	uint32_t oldPrimaryScreen = GetPrimaryMonitorID();
	std::wcout << "Aktuell konfigurierter Hauptbildschirm: " << oldPrimaryScreen + 1 << std::endl;

	// Set temporary primary monitor
	std::wcout << "Wechsle Hauptbildschirm zu " << targetMonitorID << std::endl;
	long result = SetAsPrimaryMonitor(targetMonitorID - 1);

	if (result == DISP_CHANGE_RESTART) {
		std::wcout << "Das System meldet, dass ein Neustart notwendig ist, bitte kontrollieren Sie die Monitoreinstellungen." << std::endl;
		return 0;
	}
	else if (result != DISP_CHANGE_SUCCESSFUL) {
		std::wcout << "Fehler beim Setzen der Bildschirmeinstellung. Code: " << result << std::endl;
		return 1;
	}

	// Launch process
	std::wcout << "Starte Anwendung: " << exePath << std::endl;
	if (arguments && arguments[0] != L'\0') {
		std::wcout << " mit Argumenten: " << arguments << std::endl;
	}
	int procLaunchResult = StartProcessAndWait(exePath, arguments);

	if (procLaunchResult == 0) {
		std::wcout << "Anwendung wurde geschlossen" << std::endl;
	}
	else {
		std::wcout << "Fehler beim Starten der Anwendung, Code: " << procLaunchResult << std::endl;
	}

	// Set old primary screen
	std::wcout << "Wechsle auf Hauptbildschirm " << oldPrimaryScreen + 1 << std::endl;
	result = SetAsPrimaryMonitor(oldPrimaryScreen);

	if (result == DISP_CHANGE_RESTART) {
		std::wcout << "Das System meldet, dass ein Neustart notwendig ist, bitte kontrollieren Sie die Monitoreinstellungen." << std::endl;
		return 0;
	}
	else if (result != DISP_CHANGE_SUCCESSFUL) {
		std::wcout << "Fehler beim Setzen der Bildschirmeinstellung. Code: " << result << std::endl;
		return 1;
	}

	return 0;
}

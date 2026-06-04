#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <iostream>

using namespace std;

static uint32_t GetPrimaryMonitorID() {
    DISPLAY_DEVICE device;
    device.cb = sizeof(DISPLAY_DEVICE);
    uint32_t primID;
    for (primID = 0; EnumDisplayDevices(nullptr, primID, &device, 0); primID++) {
        if (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            break;
        }
    }
    return primID;
}

static long SetAsPrimaryMonitor(uint32_t id) {
    DISPLAY_DEVICE device;
    DEVMODE deviceMode;
    device.cb = sizeof(DISPLAY_DEVICE);

    if (!EnumDisplayDevices(nullptr, id, &device, 0)) {
        std::cerr << "Fehler beim Erfassen der Display-Devices." << std::endl;
        return -1;
    }

    deviceMode.dmSize = sizeof(DEVMODE);
    if (!EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) {
        std::cerr << "Fehler beim Abrufen der Displayeinstellungen." << std::endl;
        return -1;
    }

    POINTL offset = { deviceMode.dmPosition.x, deviceMode.dmPosition.y };
    deviceMode.dmPosition.x = 0;
    deviceMode.dmPosition.y = 0;

    long setSettingsResult;

    setSettingsResult = ChangeDisplaySettingsEx(
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

int StartProcessAndWait(const LPCWSTR processPath) {
    // Process information struct
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    // Start information struct
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    // Launch process
    int procReturnCode = CreateProcess(
        processPath, // Pfad zur ausführbaren Datei
        nullptr,             // Befehlszeilenargumente
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

int main(int argc, char* argv[]) {
    cout << "Landwirtschafts-Simulator Launcher mit Hauptbildschirm-Konfiguration\n\n" << endl;

    if (argc >= 3) {
        //Get currently active primary monitor
        uint32_t oldPrimaryScreen = GetPrimaryMonitorID();
        //ID+1 because windows settings show display ids starting at 1, but the api counts starting from 0
        cout << "Aktuell konfigurierter Hauptbildschirm: " << oldPrimaryScreen+1 << endl;

        //Set temporary primary monitor
        int tempPrimID = stoi(argv[1]);
        cout << "Wechsle Hauptbildschirm zu " << tempPrimID << endl;
        //ID-1, see above
        long result = SetAsPrimaryMonitor(tempPrimID - 1);
        if (result == DISP_CHANGE_RESTART) {
            cout << "Das System meldet, dass ein Neustart notwendig ist, bitte kontrollieren Sie die Monitoreinstellungen." << endl;
            return 0;
        }
        else if (result != DISP_CHANGE_SUCCESSFUL) {
            cout << "Fehler beim Setzen der Bildschirmeinstellung. Code: " << result << endl;
            return 1;
        }

        //Set working directory
        if (argc == 4) {
            string cliworkdir = argv[3];
            wstring tempWorkdir = wstring(cliworkdir.begin(), cliworkdir.end());
            LPCWSTR workdir = tempWorkdir.c_str();
            SetCurrentDirectory(workdir);
        }

        //Launch game
        string executable = argv[2];
        wstring tempPath = wstring(executable.begin(), executable.end());
        LPCWSTR execpath = tempPath.c_str();
        cout << "Starte Anwendung " << executable << endl;
        int procLaunchResult = StartProcessAndWait(execpath);
        if (procLaunchResult == 0) {
            cout << "Anwendung wurde geschlossen" << endl;
        }
        else {
            cout << "Fehler beim Starten der Anwendung, Code: " << procLaunchResult << endl;
        }

        //Set old primary screen
        cout << "Wechsle auf Hauptbildschirm " << oldPrimaryScreen+1 << endl;
        result = SetAsPrimaryMonitor(oldPrimaryScreen);
        if (result == DISP_CHANGE_RESTART) {
            cout << "Das System meldet, dass ein Neustart notwendig ist, bitte kontrollieren Sie die Monitoreinstellungen." << endl;
            return 0;
        }
        else if (result != DISP_CHANGE_SUCCESSFUL) {
            cout << "Fehler beim Setzen der Bildschirmeinstellung. Code: " << result << endl;
            return 1;
        }

    }
    else {
        cout << "Benutzung:\n1. Parameter: ID des zu konfigurierenden Hauptbildschirms\n2. Parameter: Pfad zur .exe Anwendung zum Start des Spiels\n3. Parameter: Arbeitsverzeichnis (Working Directory)\n\n" << endl;
        cout << "Beispiel: LSScreenLauncher.exe 2 \"C:\\Program Files\\Farming Simulator 25\\x64\\FarmingSimulator25Game.exe\"" << endl;
    }
    return 0;
}

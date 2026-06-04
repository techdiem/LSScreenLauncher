#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <iostream>
#include "ApplicationLogic.h"
#include "ConfigurationGUI.h"

using namespace std;

static bool CreateAndBindConsole()
{
    if (!AllocConsole()) return false;
    FILE* fp;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    // C++-streams resync
    ios::sync_with_stdio();
    wcout.clear(); cout.clear();
    wcin.clear();  cin.clear();
    wcerr.clear(); cerr.clear();
    return true;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR lpCmdLine, int)
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    if (argc > 1) {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
			// no parent console, create a new one
            CreateAndBindConsole();
        }
        else {
            FILE* fp;
            freopen_s(&fp, "CONIN$", "r", stdin);
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            ios::sync_with_stdio();
            wcout.clear(); cout.clear();
            wcin.clear();  cin.clear();
            wcerr.clear(); cerr.clear();
        }

        if (argc >= 3) {
            wstring exePath = argv[2];
            int monitorID = _wtoi(argv[1]);
            wstring workDirStr;
            if (argc == 4) workDirStr = argv[3];
            int ret = ApplicationLogic::ExecuteWithMonitorSwitch(monitorID, exePath.c_str(),
                (argc == 4) ? workDirStr.c_str() : nullptr);

            if (argv) LocalFree(argv);
            return ret;
        }
		else if (argc < 3) {
			wcout << L"Ungültige Argumente. Verwendung: LSScreenLauncher.exe <MonitorID> <ExePath> [WorkDir]" << endl;
            cout << "Beispiel: LSScreenLauncher.exe 2 \"C:\\Program Files\\Farming Simulator 25\\x64\\FarmingSimulator25Game.exe\"" << endl;
			if (argv) LocalFree(argv);
			return -1;
		}
    }

    if (argv) LocalFree(argv);
    ConfigurationGUI::Show();
    return 0;
}

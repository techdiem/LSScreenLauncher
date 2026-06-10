#pragma once

#include <Windows.h>
#include <string>
#include <map>

class ConfigurationGUI {
public:
	static void Show();

private:
	// Control IDs
	enum ControlID {
		IDC_TITLE = 1001,
		IDC_CURRENT_LABEL = 1002,
		IDC_MONITOR_DISPLAY = 1003,
		IDC_MONITOR_LABEL = 1004,
		IDC_EXE_LABEL = 1005,
		IDC_INFO_TEXT = 1006,
		IDC_DIVIDER_1 = 1010,
		IDC_DIVIDER_2 = 1011,

		IDC_MONITOR_INPUT = 2001,
		IDC_EXE_INPUT = 2002,

		IDC_BROWSE_BTN = 3001,
		IDC_CREATE_SHORTCUT_BTN = 3002,
		IDC_REFRESH_MONITOR_ID_BTN = 3003,
		IDC_USE_ORIGINAL_ICON_CHECKBOX = 3004,
		IDC_START_MINIMIZED_CHECKBOX = 3005
	};

	// Layout constants
	static constexpr int MARGIN = 20;
	static constexpr int LABEL_HEIGHT = 20;
	static constexpr int INPUT_HEIGHT = 32;
	static constexpr int SPACING = 15;
	static constexpr int INPUT_WIDTH = 400;
	static constexpr int LABEL_WIDTH = 200;

	// Window handles
	static HWND hMainWindow;
	static HWND hMonitorIDDisplay;
	static HWND hMonitorIDInput;
	static HWND hExePathInput;
	static HWND hAppArgumentsInput;
	static HWND hCreateShortcutBtn;
	static HWND hBrowseBtn;
	static HWND hRefreshMonitorIDBtn;
	static HWND hUseOriginalIconCheckbox;
	static HWND hStartMinimizedCheckbox;
	static std::wstring g_exePath;
	static std::wstring g_appArguments;

	// Font storage
	static std::map<std::wstring, HFONT> g_fonts;

	// Window procedure
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
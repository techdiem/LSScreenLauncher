#include "ConfigurationGUI.h"
#include "GuiHelpers.h"
#include "ApplicationLogic.h"
#include "resource.h"

HWND ConfigurationGUI::hMainWindow = nullptr;
HWND ConfigurationGUI::hMonitorIDDisplay = nullptr;
HWND ConfigurationGUI::hMonitorIDInput = nullptr;
HWND ConfigurationGUI::hExePathInput = nullptr;
HWND ConfigurationGUI::hAppArgumentsInput = nullptr;
HWND ConfigurationGUI::hCreateShortcutBtn = nullptr;
HWND ConfigurationGUI::hBrowseBtn = nullptr;
HWND ConfigurationGUI::hRefreshMonitorIDBtn = nullptr;
HWND ConfigurationGUI::hUseOriginalIconCheckbox = nullptr;
HWND ConfigurationGUI::hStartMinimizedCheckbox = nullptr;
std::wstring ConfigurationGUI::g_exePath = L"";
std::wstring ConfigurationGUI::g_appArguments = L"";
std::map<std::wstring, HFONT> ConfigurationGUI::g_fonts;

LRESULT CALLBACK ConfigurationGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE: {
			SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));

			int yPos = MARGIN;

			// Get device context for DPI-aware font creation
			HDC hdcScreen = GetDC(hwnd);
			HFONT hFontTitle = GuiHelpers::CreateUIFont(16, FW_BOLD, hdcScreen);
			HFONT hFontBold = GuiHelpers::CreateUIFont(13, FW_BOLD, hdcScreen);
			HFONT hFontNormal = GuiHelpers::CreateUIFont(12, FW_NORMAL, hdcScreen);
			HFONT hFontSmall = GuiHelpers::CreateUIFont(10, FW_NORMAL, hdcScreen);
			ReleaseDC(hwnd, hdcScreen);

			// Store fonts for cleanup later
			g_fonts[L"Title"] = hFontTitle;
			g_fonts[L"Bold"] = hFontBold;
			g_fonts[L"Normal"] = hFontNormal;
			g_fonts[L"Small"] = hFontSmall;

			// Title
			GuiHelpers::CreateLabel(hwnd, L"Landwirtschafts-Simulator Launcher", MARGIN, yPos, INPUT_WIDTH, 32, hFontTitle);
			yPos += 40;

			// Current Monitor Label
			GuiHelpers::CreateLabel(hwnd, L"Aktueller Hauptbildschirm:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);

			uint32_t currentMonitor = ApplicationLogic::GetPrimaryMonitorID() + 1;
			std::wstring currentMonitorText = std::to_wstring(currentMonitor);
			hMonitorIDDisplay = GuiHelpers::CreateLabel(hwnd, currentMonitorText, MARGIN + LABEL_WIDTH + 10, yPos, 80, LABEL_HEIGHT, hFontBold);
			hRefreshMonitorIDBtn = GuiHelpers::CreateButton(hwnd, L"Aktualisieren", MARGIN + LABEL_WIDTH + 100, yPos-5, 110, INPUT_HEIGHT, hFontNormal, IDC_REFRESH_MONITOR_ID_BTN);
			yPos += LABEL_HEIGHT + SPACING + 10;

			int loadedMonitorID = currentMonitor;
			bool loadedUseOriginalIcon = false;
			bool loadedStartMinimized = true;
			if (!GuiHelpers::LoadExistingShortcutConfiguration(g_exePath, g_appArguments, loadedMonitorID, loadedUseOriginalIcon, loadedStartMinimized)) {
				g_exePath.clear();
				g_appArguments.clear();
			}

			// Divider line
			GuiHelpers::CreateDivider(hwnd, MARGIN, yPos - 5, INPUT_WIDTH, 1, IDC_DIVIDER_1);
			yPos += SPACING + 5;

			// Monitor ID Input Label
			GuiHelpers::CreateLabel(hwnd, L"Zielbildschirm-ID:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);
			hMonitorIDInput = GuiHelpers::CreateInput(hwnd, std::to_wstring(loadedMonitorID), MARGIN + LABEL_WIDTH + 10, yPos, 50, INPUT_HEIGHT, hFontNormal, ES_LEFT | ES_NUMBER);
			yPos += INPUT_HEIGHT + SPACING;

			// Exe Path Label
			GuiHelpers::CreateLabel(hwnd, L"Programm-Pfad:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);
			hExePathInput = GuiHelpers::CreateInput(hwnd, g_exePath, MARGIN + LABEL_WIDTH + 10, yPos, INPUT_WIDTH - LABEL_WIDTH - 10 - 40, INPUT_HEIGHT, hFontNormal);
			hBrowseBtn = GuiHelpers::CreateButton(hwnd, L"...", MARGIN + INPUT_WIDTH - 30, yPos, 30, INPUT_HEIGHT, hFontNormal, IDC_BROWSE_BTN);
			yPos += INPUT_HEIGHT + 8;

			// App Arguments Label
			GuiHelpers::CreateLabel(hwnd, L"Programm-Argumente:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);
			hAppArgumentsInput = GuiHelpers::CreateInput(hwnd, g_appArguments, MARGIN + LABEL_WIDTH + 10, yPos, INPUT_WIDTH - LABEL_WIDTH - 10, INPUT_HEIGHT, hFontNormal);
			yPos += INPUT_HEIGHT + 8;

			hUseOriginalIconCheckbox = GuiHelpers::CreateCheckbox(hwnd, L"Originales Programmicon verwenden", MARGIN, yPos, 300, LABEL_HEIGHT + 4, hFontNormal, IDC_USE_ORIGINAL_ICON_CHECKBOX);
			if (loadedUseOriginalIcon) {
				SendMessage(hUseOriginalIconCheckbox, BM_SETCHECK, BST_CHECKED, 0);
			}
			yPos += LABEL_HEIGHT + SPACING;
			hStartMinimizedCheckbox = GuiHelpers::CreateCheckbox(hwnd, L"Minimiert starten", MARGIN, yPos, 300, LABEL_HEIGHT + 4, hFontNormal, IDC_START_MINIMIZED_CHECKBOX);
			if (loadedStartMinimized) {
				SendMessage(hStartMinimizedCheckbox, BM_SETCHECK, BST_CHECKED, 0);
			}
			yPos += LABEL_HEIGHT + SPACING;


			// Divider line
			GuiHelpers::CreateDivider(hwnd, MARGIN, yPos - 5, INPUT_WIDTH, 1, IDC_DIVIDER_2);
			yPos += SPACING + 5;

			// Create Shortcut Button
			hCreateShortcutBtn = GuiHelpers::CreateButton(hwnd, L"Starter erstellen", MARGIN, yPos, 200, INPUT_HEIGHT + 5, hFontNormal, IDC_CREATE_SHORTCUT_BTN);

			// Info text
			GuiHelpers::CreateLabel(hwnd, L"Dies erstellt einen Starter auf dem Desktop.", MARGIN, yPos + INPUT_HEIGHT + 8, INPUT_WIDTH, LABEL_HEIGHT, hFontSmall);

			break;
			}

		case WM_COMMAND: {
			int wmId = LOWORD(wParam);

			if (wmId == IDC_BROWSE_BTN) {  // Browse button
				std::wstring filePath = GuiHelpers::BrowseForFile(hwnd);
				if (!filePath.empty()) {
					g_exePath = filePath;
					SetWindowText(hExePathInput, filePath.c_str());
				}
				return 0;
			}
			else if (wmId == IDC_REFRESH_MONITOR_ID_BTN) {
				uint32_t currentMonitor = ApplicationLogic::GetPrimaryMonitorID() + 1;
				std::wstring currentMonitorText = std::to_wstring(currentMonitor);
				SetWindowText(hMonitorIDDisplay, currentMonitorText.c_str());
				return 0;
			}
			else if (wmId == IDC_CREATE_SHORTCUT_BTN) {  // Create Shortcut button
				wchar_t buffer[256] = { 0 };
				GetWindowText(hMonitorIDInput, buffer, 256);
				std::wstring monitorIDStr(buffer);

				wchar_t appArgsBuffer[1024] = { 0 };
				GetWindowText(hAppArgumentsInput, appArgsBuffer, 1024);
				g_appArguments = appArgsBuffer;

				if (monitorIDStr.empty() || g_exePath.empty()) {
					MessageBox(hwnd, L"Bitte tragen Sie alle Felder ein.", L"Eingabefehler", MB_OK | MB_ICONWARNING);
					return 0;
				}

				int monitorID = _wtoi(monitorIDStr.c_str());
				if (monitorID <= 0) {
					MessageBox(hwnd, L"Die Bildschirm-ID muss eine positive Zahl sein.", L"Eingabefehler", MB_OK | MB_ICONWARNING);
					return 0;
				}

				bool useOriginalIcon = SendMessage(hUseOriginalIconCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
				bool startMinimized = SendMessage(hStartMinimizedCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
				GuiHelpers::CreateDesktopShortcut(hwnd, g_exePath, g_appArguments, monitorID, useOriginalIcon, startMinimized);
				return 0;
			}
			break;
		}

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;

		case WM_DESTROY:
			GuiHelpers::CleanupFonts(g_fonts);
			PostQuitMessage(0);
			return 0;

		case WM_CTLCOLORSTATIC: {
			HDC hdcStatic = (HDC)wParam;
			SetBkMode(hdcStatic, TRANSPARENT);
			SetTextColor(hdcStatic, RGB(0, 0, 0));
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}

		case WM_CTLCOLOREDIT: {
			HDC hdcEdit = (HDC)wParam;
			SetBkColor(hdcEdit, RGB(255, 255, 255));
			SetTextColor(hdcEdit, RGB(0, 0, 0));
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void ConfigurationGUI::Show() {
	// Initialize COM for shell links
	CoInitialize(nullptr);

	// Register window class (explicit Unicode)
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.lpszClassName = L"LSScreenLauncherConfigClass";
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hIcon = (HICON)LoadImageW(wcex.hInstance, MAKEINTRESOURCEW(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	wcex.hIconSm = wcex.hIcon;

	RegisterClassExW(&wcex);

	// Create window
	hMainWindow = CreateWindowEx(
		WS_EX_APPWINDOW,
		L"LSScreenLauncherConfigClass",
		L"Landwirtschafts-Simulator Launcher Konfiguration",
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		470, 460,
		nullptr, nullptr,
		GetModuleHandle(nullptr),
		nullptr
	);

	if (!hMainWindow) {
		MessageBox(nullptr, L"Fehler beim Erstellen des Fensters.", L"Fehler", MB_OK | MB_ICONERROR);
		return;
	}

	// Center window on screen
	RECT rect;
	GetWindowRect(hMainWindow, &rect);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	SetWindowPos(hMainWindow, nullptr,
		(screenWidth - (rect.right - rect.left)) / 2,
		(screenHeight - (rect.bottom - rect.top)) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);

	ShowWindow(hMainWindow, SW_SHOW);
	UpdateWindow(hMainWindow);

	// Message loop
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
}

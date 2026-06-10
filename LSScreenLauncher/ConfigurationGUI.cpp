#include "ConfigurationGUI.h"
#include "ApplicationLogic.h"
#include <shlobj.h>
#include <comdef.h>
#include "resource.h"

HWND ConfigurationGUI::hMainWindow = nullptr;
HWND ConfigurationGUI::hMonitorIDDisplay = nullptr;
HWND ConfigurationGUI::hMonitorIDInput = nullptr;
HWND ConfigurationGUI::hExePathInput = nullptr;
HWND ConfigurationGUI::hCreateShortcutBtn = nullptr;
HWND ConfigurationGUI::hBrowseBtn = nullptr;
HWND ConfigurationGUI::hRefreshMonitorIDBtn = nullptr;
HWND ConfigurationGUI::hUseOriginalIconCheckbox = nullptr;
HWND ConfigurationGUI::hStartMinimizedCheckbox = nullptr;
std::wstring ConfigurationGUI::g_exePath = L"";
std::map<std::wstring, HFONT> ConfigurationGUI::g_fonts;

HFONT ConfigurationGUI::CreateUIFont(int ptSize, int weight, HDC hdc) {
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	int heightInPixels = -MulDiv(ptSize, dpi, 72);
	HFONT hFont = CreateFontW(heightInPixels, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
	return hFont;
}

HWND ConfigurationGUI::CreateLabel(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont) {
	HWND hLabel = CreateWindowEx(0, L"STATIC", text.c_str(),
		WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, width, height,
		hParent, nullptr, GetModuleHandle(nullptr), nullptr);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hLabel;
}

HWND ConfigurationGUI::CreateInput(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, DWORD dwStyle) {
	HWND hInput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", text.c_str(),
		WS_CHILD | WS_VISIBLE | dwStyle, x, y, width, height,
		hParent, nullptr, GetModuleHandle(nullptr), nullptr);
	SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hInput;
}

HWND ConfigurationGUI::CreateButton(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID) {
	HWND hButton = CreateWindowEx(0, L"BUTTON", text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, width, height,
		hParent, (HMENU)(INT_PTR)controlID, GetModuleHandle(nullptr), nullptr);
	SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hButton;
}

HWND ConfigurationGUI::CreateCheckbox(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID) {
	HWND hCheckbox = CreateWindowEx(0, L"BUTTON", text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, width, height,
		hParent, (HMENU)(INT_PTR)controlID, GetModuleHandle(nullptr), nullptr);
	SendMessage(hCheckbox, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hCheckbox;
}

HWND ConfigurationGUI::CreateDivider(HWND hParent, int x, int y, int width, int height, int controlID) {
	return CreateWindowEx(0, L"STATIC", L"",
		WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, x, y, width, height,
		hParent, (HMENU)controlID, GetModuleHandle(nullptr), nullptr);
}

void ConfigurationGUI::CleanupFonts() {
	for (auto& pair : g_fonts) {
		if (pair.second) {
			DeleteObject(pair.second);
		}
	}
	g_fonts.clear();
}

bool ConfigurationGUI::CreateDesktopShortcut(const std::wstring& exePath, int monitorID, bool useOriginalIcon, bool startMinimized) {
	// Get desktop path
	wchar_t desktopPath[MAX_PATH];
	if (FAILED(SHGetFolderPath(nullptr, CSIDL_DESKTOP, nullptr, 0, desktopPath))) {
		MessageBox(hMainWindow, L"Fehler beim Abrufen des Desktops.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	std::wstring shortcutPath = desktopPath;
	shortcutPath += L"\\LSScreenLauncher.lnk";

	// Get the exe directory as working directory
	std::wstring workDir = exePath;
	size_t lastBackslash = workDir.find_last_of(L"\\");
	if (lastBackslash != std::wstring::npos) {
		workDir = workDir.substr(0, lastBackslash);
	}

	// Get the LSScreenLauncher.exe path
	wchar_t exeDir[MAX_PATH];
	GetModuleFileName(nullptr, exeDir, MAX_PATH);
	std::wstring launcherPath = exeDir;

	// Create COM objects for shortcut
	IShellLink* psl = nullptr;
	IPersistFile* ppf = nullptr;

	if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl))) {
		MessageBox(hMainWindow, L"Fehler beim Erstellen des Shell Links.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	// Set shortcut properties
	psl->SetPath(launcherPath.c_str());

	// Build arguments string
	std::wstring arguments = std::to_wstring(monitorID) + L" \"" + exePath + L"\"";
	psl->SetArguments(arguments.c_str());

	psl->SetWorkingDirectory(workDir.c_str());
	psl->SetDescription(L"Landwirtschafts-Simulator mit Bildschirmkonfiguration");
	psl->SetIconLocation(useOriginalIcon ? exePath.c_str() : launcherPath.c_str(), 0);
	psl->SetShowCmd(startMinimized ? SW_SHOWMINNOACTIVE : SW_NORMAL);

	// Save the shortcut
	if (FAILED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
		psl->Release();
		MessageBox(hMainWindow, L"Fehler beim Speichern des Links.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	if (FAILED(ppf->Save(shortcutPath.c_str(), TRUE))) {
		ppf->Release();
		psl->Release();
		MessageBox(hMainWindow, L"Fehler beim Speichern des Links auf dem Desktop.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	ppf->Release();
	psl->Release();

	MessageBox(hMainWindow, L"Link erfolgreich auf dem Desktop erstellt!", L"Erfolg", MB_OK | MB_ICONINFORMATION);
	return true;
}

std::wstring ConfigurationGUI::BrowseForFile() {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWindow;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = L"Pfad zur Anwendung angeben";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = nullptr;

	if (GetOpenFileName(&ofn)) {
		return std::wstring(szFile);
	}
	return L"";
}

LRESULT CALLBACK ConfigurationGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE: {
			SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));

			int yPos = MARGIN;

			// Get device context for DPI-aware font creation
			HDC hdcScreen = GetDC(hwnd);
			HFONT hFontTitle = CreateUIFont(16, FW_BOLD, hdcScreen);
			HFONT hFontBold = CreateUIFont(13, FW_BOLD, hdcScreen);
			HFONT hFontNormal = CreateUIFont(12, FW_NORMAL, hdcScreen);
			HFONT hFontSmall = CreateUIFont(10, FW_NORMAL, hdcScreen);
			ReleaseDC(hwnd, hdcScreen);

			// Store fonts for cleanup later
			g_fonts[L"Title"] = hFontTitle;
			g_fonts[L"Bold"] = hFontBold;
			g_fonts[L"Normal"] = hFontNormal;
			g_fonts[L"Small"] = hFontSmall;

			// Title
			CreateLabel(hwnd, L"Landwirtschafts-Simulator Launcher", MARGIN, yPos, INPUT_WIDTH, 32, hFontTitle);
			yPos += 40;

			// Current Monitor Label
			CreateLabel(hwnd, L"Aktueller Hauptbildschirm:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);

			uint32_t currentMonitor = ApplicationLogic::GetPrimaryMonitorID() + 1;
			std::wstring currentMonitorText = std::to_wstring(currentMonitor);
			hMonitorIDDisplay = CreateLabel(hwnd, currentMonitorText, MARGIN + LABEL_WIDTH + 10, yPos, 80, LABEL_HEIGHT, hFontBold);
			hRefreshMonitorIDBtn = CreateButton(hwnd, L"Aktualisieren", MARGIN + LABEL_WIDTH + 100, yPos-5, 110, INPUT_HEIGHT, hFontNormal, IDC_REFRESH_MONITOR_ID_BTN);
			yPos += LABEL_HEIGHT + SPACING + 10;

			// Divider line
			CreateDivider(hwnd, MARGIN, yPos - 5, INPUT_WIDTH, 1, IDC_DIVIDER_1);
			yPos += SPACING + 5;

			// Monitor ID Input Label
			CreateLabel(hwnd, L"Zielbildschirm-ID:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);
			hMonitorIDInput = CreateInput(hwnd, L"2", MARGIN + LABEL_WIDTH + 10, yPos, 50, INPUT_HEIGHT, hFontNormal, ES_LEFT | ES_NUMBER);
			yPos += INPUT_HEIGHT + SPACING;

			// Exe Path Label
			CreateLabel(hwnd, L"Programm-Pfad:", MARGIN, yPos, LABEL_WIDTH, LABEL_HEIGHT, hFontNormal);
			hExePathInput = CreateInput(hwnd, L"", MARGIN + LABEL_WIDTH + 10, yPos, INPUT_WIDTH - LABEL_WIDTH - 10 - 40, INPUT_HEIGHT, hFontNormal);
			hBrowseBtn = CreateButton(hwnd, L"...", MARGIN + INPUT_WIDTH - 30, yPos, 30, INPUT_HEIGHT, hFontNormal, IDC_BROWSE_BTN);
			yPos += INPUT_HEIGHT + 8;

			hUseOriginalIconCheckbox = CreateCheckbox(hwnd, L"Originales Programmicon verwenden", MARGIN, yPos, 300, LABEL_HEIGHT + 4, hFontNormal, IDC_USE_ORIGINAL_ICON_CHECKBOX);
			yPos += LABEL_HEIGHT + SPACING;
			hStartMinimizedCheckbox = CreateCheckbox(hwnd, L"Minimiert starten", MARGIN, yPos, 300, LABEL_HEIGHT + 4, hFontNormal, IDC_START_MINIMIZED_CHECKBOX);
			SendMessage(hStartMinimizedCheckbox, BM_SETCHECK, BST_CHECKED, 0);
			yPos += LABEL_HEIGHT + SPACING;


			// Divider line
			CreateDivider(hwnd, MARGIN, yPos - 5, INPUT_WIDTH, 1, IDC_DIVIDER_2);
			yPos += SPACING + 5;

			// Create Shortcut Button
			hCreateShortcutBtn = CreateButton(hwnd, L"Starter erstellen", MARGIN, yPos, 200, INPUT_HEIGHT + 5, hFontNormal, IDC_CREATE_SHORTCUT_BTN);

			// Info text
			CreateLabel(hwnd, L"Dies erstellt einen Starter auf dem Desktop.", MARGIN, yPos + INPUT_HEIGHT + 8, INPUT_WIDTH, LABEL_HEIGHT, hFontSmall);

			break;
			}

		case WM_COMMAND: {
			int wmId = LOWORD(wParam);

			if (wmId == IDC_BROWSE_BTN) {  // Browse button
				std::wstring filePath = BrowseForFile();
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
				CreateDesktopShortcut(g_exePath, monitorID, useOriginalIcon, startMinimized);
				return 0;
			}
			break;
		}

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;

		case WM_DESTROY:
			CleanupFonts();
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
		470, 410,
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

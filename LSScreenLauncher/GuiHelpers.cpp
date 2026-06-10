#include "GuiHelpers.h"

#include <shlobj.h>
#include <shellapi.h>
#include <comdef.h>
#include <commdlg.h>

namespace {
	std::wstring Unquote(const std::wstring& value) {
		if (value.size() >= 2 && value.front() == L'"' && value.back() == L'"') {
			return value.substr(1, value.size() - 2);
		}
		return value;
	}
}

std::wstring GuiHelpers::GetDesktopShortcutPath() {
	wchar_t desktopPath[MAX_PATH] = { 0 };
	if (FAILED(SHGetFolderPath(nullptr, CSIDL_DESKTOP, nullptr, 0, desktopPath))) {
		return L"";
	}

	std::wstring shortcutPath = desktopPath;
	shortcutPath += L"\\LSScreenLauncher.lnk";
	return shortcutPath;
}

bool GuiHelpers::LoadExistingShortcutConfiguration(std::wstring& exePath, std::wstring& appArguments, int& monitorID, bool& useOriginalIcon, bool& startMinimized) {
	std::wstring shortcutPath = GetDesktopShortcutPath();
	if (shortcutPath.empty()) {
		return false;
	}

	IShellLink* psl = nullptr;
	IPersistFile* ppf = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
	if (FAILED(hr)) {
		return false;
	}

	hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (FAILED(hr)) {
		psl->Release();
		return false;
	}

	hr = ppf->Load(shortcutPath.c_str(), STGM_READ);
	if (FAILED(hr)) {
		ppf->Release();
		psl->Release();
		return false;
	}

	wchar_t arguments[4096] = { 0 };
	wchar_t iconPath[MAX_PATH] = { 0 };
	int iconIndex = 0;
	int showCmd = SW_SHOWNORMAL;
	wchar_t targetPath[MAX_PATH] = { 0 };

	psl->GetArguments(arguments, 4096);
	psl->GetIconLocation(iconPath, MAX_PATH, &iconIndex);
	psl->GetShowCmd(&showCmd);
	psl->GetPath(targetPath, MAX_PATH, nullptr, SLGP_RAWPATH);

	ppf->Release();
	psl->Release();

	std::wstring argumentsText(arguments);
	size_t firstSpace = argumentsText.find(L' ');
	if (firstSpace == std::wstring::npos) {
		return false;
	}

	std::wstring monitorText = argumentsText.substr(0, firstSpace);
	try {
		monitorID = std::stoi(monitorText);
	}
	catch (...) {
		return false;
	}

	std::wstring remainingArguments = argumentsText.substr(firstSpace + 1);
	size_t secondSpace = remainingArguments.find(L' ');
	if (secondSpace == std::wstring::npos) {
		return false;
	}

	std::wstring exePathText = Unquote(remainingArguments.substr(0, secondSpace));
	std::wstring appArgumentsText = Unquote(remainingArguments.substr(secondSpace + 1));
	if (exePathText.empty()) {
		return false;
	}

	exePath = exePathText;
	appArguments = appArgumentsText;
	useOriginalIcon = (_wcsicmp(iconPath, exePath.c_str()) == 0);
	startMinimized = (showCmd == SW_SHOWMINNOACTIVE);
	return true;
}

HFONT GuiHelpers::CreateUIFont(int ptSize, int weight, HDC hdc) {
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	int heightInPixels = -MulDiv(ptSize, dpi, 72);
	HFONT hFont = CreateFontW(heightInPixels, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
	return hFont;
}

HWND GuiHelpers::CreateLabel(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont) {
	HWND hLabel = CreateWindowEx(0, L"STATIC", text.c_str(),
		WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, width, height,
		hParent, nullptr, GetModuleHandle(nullptr), nullptr);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hLabel;
}

HWND GuiHelpers::CreateInput(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, DWORD dwStyle) {
	HWND hInput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", text.c_str(),
		WS_CHILD | WS_VISIBLE | dwStyle, x, y, width, height,
		hParent, nullptr, GetModuleHandle(nullptr), nullptr);
	SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hInput;
}

HWND GuiHelpers::CreateButton(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID) {
	HWND hButton = CreateWindowEx(0, L"BUTTON", text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, width, height,
		hParent, (HMENU)(INT_PTR)controlID, GetModuleHandle(nullptr), nullptr);
	SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hButton;
}

HWND GuiHelpers::CreateCheckbox(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID) {
	HWND hCheckbox = CreateWindowEx(0, L"BUTTON", text.c_str(),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, width, height,
		hParent, (HMENU)(INT_PTR)controlID, GetModuleHandle(nullptr), nullptr);
	SendMessage(hCheckbox, WM_SETFONT, (WPARAM)hFont, FALSE);
	return hCheckbox;
}

HWND GuiHelpers::CreateDivider(HWND hParent, int x, int y, int width, int height, int controlID) {
	return CreateWindowEx(0, L"STATIC", L"",
		WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, x, y, width, height,
		hParent, (HMENU)controlID, GetModuleHandle(nullptr), nullptr);
}

void GuiHelpers::CleanupFonts(std::map<std::wstring, HFONT>& fonts) {
	for (auto& pair : fonts) {
		if (pair.second) {
			DeleteObject(pair.second);
		}
	}
	fonts.clear();
}

bool GuiHelpers::CreateDesktopShortcut(HWND owner, const std::wstring& exePath, const std::wstring& appArguments, int monitorID, bool useOriginalIcon, bool startMinimized) {
	std::wstring shortcutPath = GetDesktopShortcutPath();

	std::wstring workDir = exePath;
	size_t lastBackslash = workDir.find_last_of(L"\\");
	if (lastBackslash != std::wstring::npos) {
		workDir = workDir.substr(0, lastBackslash);
	}

	wchar_t exeDir[MAX_PATH];
	GetModuleFileName(nullptr, exeDir, MAX_PATH);
	std::wstring launcherPath = exeDir;

	IShellLink* psl = nullptr;
	IPersistFile* ppf = nullptr;

	if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl))) {
		MessageBox(owner, L"Fehler beim Erstellen des Shell Links.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	psl->SetPath(launcherPath.c_str());

	std::wstring arguments = std::to_wstring(monitorID) + L" \"" + exePath + L"\"";
	if (!appArguments.empty()) {
		arguments += L" \"" + appArguments + L"\"";
	}
	psl->SetArguments(arguments.c_str());

	psl->SetWorkingDirectory(workDir.c_str());
	psl->SetDescription(L"Landwirtschafts-Simulator mit Bildschirmkonfiguration");
	psl->SetIconLocation(useOriginalIcon ? exePath.c_str() : launcherPath.c_str(), 0);
	psl->SetShowCmd(startMinimized ? SW_SHOWMINNOACTIVE : SW_NORMAL);

	if (FAILED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
		psl->Release();
		MessageBox(owner, L"Fehler beim Speichern des Links.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	if (FAILED(ppf->Save(shortcutPath.c_str(), TRUE))) {
		ppf->Release();
		psl->Release();
		MessageBox(owner, L"Fehler beim Speichern des Links auf dem Desktop.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	ppf->Release();
	psl->Release();

	MessageBox(owner, L"Link erfolgreich auf dem Desktop erstellt!", L"Erfolg", MB_OK | MB_ICONINFORMATION);
	return true;
}

std::wstring GuiHelpers::BrowseForFile(HWND owner) {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;
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

#pragma once

#include <Windows.h>
#include <string>
#include <map>

class GuiHelpers {
public:
	static HFONT CreateUIFont(int ptSize, int weight, HDC hdc);
	static HWND CreateLabel(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont);
	static HWND CreateInput(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, DWORD dwStyle = ES_LEFT | ES_AUTOHSCROLL);
	static HWND CreateButton(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID);
	static HWND CreateCheckbox(HWND hParent, const std::wstring& text, int x, int y, int width, int height, HFONT hFont, int controlID);
	static HWND CreateDivider(HWND hParent, int x, int y, int width, int height, int controlID);
	static void CleanupFonts(std::map<std::wstring, HFONT>& fonts);

	static bool LoadExistingShortcutConfiguration(std::wstring& exePath, std::wstring& appArguments, int& monitorID, bool& useOriginalIcon, bool& startMinimized);
	static bool CreateDesktopShortcut(HWND owner, const std::wstring& exePath, const std::wstring& appArguments, int monitorID, bool useOriginalIcon, bool startMinimized);
	static std::wstring BrowseForFile(HWND owner);

private:
	static std::wstring GetDesktopShortcutPath();
};

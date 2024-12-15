#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>
#include <map>

#pragma comment(lib, "gdiplus.lib")

// Global variables
HINSTANCE hInst;
NOTIFYICONDATA nid;
HMENU hMenu;
std::map<std::wstring, std::wstring> config;
std::wofstream logFile;

const CLSID pngClsid = {
    0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

void LoadConfig(const std::wstring& filePath) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        MessageBox(NULL, L"Failed to open configuration file cfg.txt", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), iswspace), line.end());
        if (line.empty() || line[0] == L'#') continue;

        size_t pos = line.find(L'=');
        if (pos != std::wstring::npos) {
            std::wstring key = line.substr(0, pos);
            std::wstring value = line.substr(pos + 1);
            config[key] = value;
        }
    }

    file.close();
}

void SaveScreenshotToPNG(const std::wstring& filePath) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hdcMem, hOldBitmap);

    Gdiplus::Bitmap bitmap(hBitmap, NULL);
    bitmap.Save(filePath.c_str(), &pngClsid, NULL);

    DeleteObject(hBitmap);
    DeleteObject(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    Gdiplus::GdiplusShutdown(gdiplusToken);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_USER + 1:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
        }
        break;
    case WM_HOTKEY:
        if (wParam == 1) {
            std::wstring savePath = config[L"folder"];
            CreateDirectory(savePath.c_str(), NULL);

            std::time_t t = std::time(nullptr);
            std::tm tm;
            localtime_s(&tm, &t);
            wchar_t fileName[256];
            wcsftime(fileName, sizeof(fileName), config[L"filename"].c_str(), &tm);

            std::wstring fullPath = savePath + L"\\" + fileName;
            SaveScreenshotToPNG(fullPath);

            logFile << L"Screenshot saved: " << fullPath << std::endl;
            logFile.flush(); // Ensure the log is written to disk
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    hInst = hInstance;

    LoadConfig(L"cfg.txt");

    if (config.find(L"hotkey") == config.end()) config[L"hotkey"] = L"Print Screen";
    if (config.find(L"folder") == config.end()) config[L"folder"] = L"screenshots";
    if (config.find(L"filename") == config.end()) config[L"filename"] = L"%Y%m%d_%H%M%S.png";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ntshotClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"ntshotClass", L"ntshot", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, L"Exit");

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = (HICON)LoadImage(hInstance, L"ntshot.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    wcscpy_s(nid.szTip, L"ntshot");
    Shell_NotifyIcon(NIM_ADD, &nid);

    RegisterHotKey(hwnd, 1, 0, VK_SNAPSHOT);

    logFile.open("log.txt", std::ios::app);
    if (!logFile.is_open()) {
        MessageBox(NULL, L"Failed to open log file log.txt", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    logFile << L"Program started" << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    logFile << L"Program ended" << std::endl;
    logFile.close();

    DestroyMenu(hMenu);

    return (int)msg.wParam;
}
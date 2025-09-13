/* MIT License

Copyright (c) 2024-2025 Stefan-Mihai MOGA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

/**
 * @file DemoApp.cpp
 * @brief Entry point and main logic for the DemoApp Windows application.
 *        Demonstrates usage of the genUp4win update library.
 */

#include "pch.h"
#include "framework.h"
#include "DemoApp.h"
#include "Commctrl.h"

#define GENUP4WIN_EXPORTS
#include "../genUp4win.h"
#if _WIN64
#ifdef _DEBUG
#pragma comment(lib, "../x64/Debug/genUp4win.lib")
#else
#pragma comment(lib, "../x64/Release/genUp4win.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "../Debug/genUp4win.lib")
#else
#pragma comment(lib, "../Release/genUp4win.lib")
#endif
#endif

#define MAX_LOADSTRING 100

// Global Variables
HINSTANCE hInst;                                ///< Current application instance
WCHAR szTitle[MAX_LOADSTRING];                  ///< The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            ///< The main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutCallback(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    UpdateCallback(HWND, UINT, WPARAM, LPARAM);

/**
 * @brief Retrieves the full path of the current module (executable).
 * @param pdwLastError Optional pointer to receive the last error code.
 * @return The full path as a CString, or empty string on failure.
 */
CString GetModuleFileName(_Inout_opt_ DWORD* pdwLastError = nullptr)
{
    CString strModuleFileName;
    DWORD dwSize{ _MAX_PATH };
    while (true)
    {
        TCHAR* pszModuleFileName{ strModuleFileName.GetBuffer(dwSize) };
        const DWORD dwResult{ ::GetModuleFileName(nullptr, pszModuleFileName, dwSize) };
        if (dwResult == 0)
        {
            if (pdwLastError != nullptr)
                *pdwLastError = GetLastError();
            strModuleFileName.ReleaseBuffer(0);
            return CString{};
        }
        else if (dwResult < dwSize)
        {
            if (pdwLastError != nullptr)
                *pdwLastError = ERROR_SUCCESS;
            strModuleFileName.ReleaseBuffer(dwResult);
            return strModuleFileName;
        }
        else if (dwResult == dwSize)
        {
            strModuleFileName.ReleaseBuffer(0);
            dwSize *= 2;
        }
    }
}

/**
 * @brief The main entry point for the application.
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Write a configuration file for the updater (for demonstration)
    CString strFullPath{ GetModuleFileName() };
    WriteConfigFile(strFullPath.GetString(), _T("https://www.moga.doctor/freeware/IntelliEditSetup.msi"));

    // TODO: Please upload the configuration file to your Web Server.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DEMOAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMOAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

/**
 * @brief Registers the main window class.
 * @param hInstance Application instance handle.
 * @return The atom value for the registered class.
 */
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMOAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEMOAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

/**
 * @brief Saves instance handle and creates main window.
 * @param hInstance Application instance handle.
 * @param nCmdShow Show command for the window.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

/**
 * @brief Processes messages for the main window.
 * @param hWnd Window handle.
 * @param message Message identifier.
 * @param wParam Additional message information.
 * @param lParam Additional message information.
 * @return LRESULT result of message processing.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case ID_FILE_CHECK_FOR_UPDATES:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_CHECK_FOR_UPDATES), hWnd, UpdateCallback);
                    break;
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutCallback);
                    break;
                case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
       }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            UNREFERENCED_PARAMETER(hdc);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/**
 * @brief Centers a window relative to its parent and ensures it stays on screen.
 * @param hwndWindow Handle to the window to center.
 * @return TRUE if centered, FALSE otherwise.
 */
BOOL CenterWindow(HWND hwndWindow)
{
    HWND hwndParent;
    RECT rectWindow, rectParent;

    // make the window relative to its parent
    if ((hwndParent = GetParent(hwndWindow)) != NULL)
    {
        GetWindowRect(hwndWindow, &rectWindow);
        GetWindowRect(hwndParent, &rectParent);

        int nWidth = rectWindow.right - rectWindow.left;
        int nHeight = rectWindow.bottom - rectWindow.top;

        int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
        int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        // make sure that the dialog box never moves outside of the screen
        if (nX < 0) nX = 0;
        if (nY < 0) nY = 0;
        if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
        if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;

        MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Message handler for the About dialog box.
 * @param hDlg Dialog handle.
 * @param message Message identifier.
 * @param wParam Additional message information.
 * @param lParam Additional message information.
 * @return TRUE if handled, FALSE otherwise.
 */
INT_PTR CALLBACK AboutCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            return (INT_PTR)TRUE;
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    }
    return (INT_PTR)FALSE;
}

HWND hwndDialog = nullptr;

/**
 * @brief Callback function for UI updates during update checking.
 * @param status Status code.
 * @param strMessage Status message to display.
 */
void UI_Callback(int status, const std::wstring& strMessage)
{
	UNREFERENCED_PARAMETER(status);
    SetWindowText(GetDlgItem(hwndDialog, IDC_STATUS), strMessage.c_str());
    UpdateWindow(GetDlgItem(hwndDialog, IDC_STATUS));
}

bool g_bThreadRunning = false;
bool g_bNewUpdateFound = false;

/**
 * @brief Thread procedure for checking for updates.
 * @param lpParam Unused parameter.
 * @return DWORD exit code.
 */
DWORD WINAPI UpdateThreadProc(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    g_bThreadRunning = true;
    CString strFullPath{ GetModuleFileName() };
    g_bNewUpdateFound = CheckForUpdates(strFullPath.GetString(), _T("https://www.moga.doctor/freeware/genUp4win.xml"), UI_Callback);
    g_bThreadRunning = false;

    ::ExitThread(0);
    // return 0;
}

DWORD m_nUpdateThreadID = 0;
HANDLE m_hUpdateThread = 0;
UINT_PTR m_nTimerID = 0;

/**
 * @brief Message handler for the "Check for updates" dialog box.
 *        Launches a thread to check for updates and closes the dialog when done.
 * @param hDlg Dialog handle.
 * @param message Message identifier.
 * @param wParam Additional message information.
 * @param lParam Additional message information.
 * @return TRUE if handled, FALSE otherwise.
 */
INT_PTR CALLBACK UpdateCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            hwndDialog = hDlg; // used in UI_Callback
            CenterWindow(hDlg);
            // SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)30);
            m_hUpdateThread = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)UpdateThreadProc, hDlg, 0, &m_nUpdateThreadID);
            m_nTimerID = SetTimer(hDlg, 0x1234, 100, nullptr);
            return (INT_PTR)TRUE;
        case WM_TIMER:
            if (!g_bThreadRunning)
            {
                EndDialog(hDlg, IDCANCEL);
                if (g_bNewUpdateFound)
                {
                    PostQuitMessage(0);
                }
            }
            return 0;
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    }
    return (INT_PTR)FALSE;
}

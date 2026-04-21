/* MIT License

Copyright (c) 2024-2026 Stefan-Mihai MOGA

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
    // This creates/updates the local XML configuration file that the updater will use
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

    // Load keyboard accelerators for menu shortcuts
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMOAPP));

    MSG msg;

    // Main message loop:
    // Retrieves and processes messages until WM_QUIT is received
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // Check if message is a keyboard accelerator
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            // Translate virtual-key messages into character messages
            TranslateMessage(&msg);
            // Dispatch message to window procedure
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

    // Window will redraw when resized horizontally or vertically
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    // Set the window procedure callback
    wcex.lpfnWndProc    = WndProc;
    // No extra bytes for class or window instance
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    // Load application icon
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMOAPP));
    // Set cursor to standard arrow
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    // Set background color to system window color
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    // Attach menu resource
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEMOAPP);
    wcex.lpszClassName  = szWindowClass;
    // Load small icon for taskbar
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
   // Store instance handle in our global variable
   hInst = hInstance;

   // Create the main application window with default size and position
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   // Check if window creation succeeded
   if (!hWnd)
   {
      return FALSE;
   }

   // Display the window with the specified show command
   ShowWindow(hWnd, nCmdShow);
   // Send WM_PAINT message to update window content
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
            // Extract menu item ID from wParam
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case ID_FILE_CHECK_FOR_UPDATES:
                    // Show the update checking dialog
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_CHECK_FOR_UPDATES), hWnd, UpdateCallback);
                    break;
                case IDM_ABOUT:
                    // Show the About dialog
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutCallback);
                    break;
                case IDM_EXIT:
                    // Close the application
                    DestroyWindow(hWnd);
                    break;
                default:
                    // Let Windows handle any unprocessed commands
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
       }
        case WM_PAINT:
        {
            // Handle window painting
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            UNREFERENCED_PARAMETER(hdc);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            // Post a quit message to exit the application
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

    // Get parent window and center the child window relative to it
    if ((hwndParent = GetParent(hwndWindow)) != NULL)
    {
        // Get dimensions of both windows
        GetWindowRect(hwndWindow, &rectWindow);
        GetWindowRect(hwndParent, &rectParent);

        // Calculate window dimensions
        int nWidth = rectWindow.right - rectWindow.left;
        int nHeight = rectWindow.bottom - rectWindow.top;

        // Calculate centered position within parent
        int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
        int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

        // Get screen dimensions for boundary checking
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Make sure that the dialog box never moves outside of the screen
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
            // Center the About dialog on screen when initialized
            CenterWindow(hDlg);
            return (INT_PTR)TRUE;
        case WM_COMMAND:
        {
            // Handle OK or Cancel button clicks
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

// Global dialog handle used by the update callback to update UI
HWND hwndDialog = nullptr;

/**
 * @brief Callback function for UI updates during update checking.
 * @param status Status code.
 * @param strMessage Status message to display.
 * @param nProgress Progress percentage (0-100).
 */
void UI_Callback(int status, const std::wstring& strMessage, const int& nProgress)
{
    UNREFERENCED_PARAMETER(status);
    // Update the status text in the update dialog
    SetWindowText(GetDlgItem(hwndDialog, IDC_STATUS), strMessage.c_str());
    // Force immediate update of the status control
    UpdateWindow(GetDlgItem(hwndDialog, IDC_STATUS));
	// Log progress percentage to debug output for monitoring
	// This can be extended to update a progress bar control in the UI
	CString strProgress;
	strProgress.Format(_T("Progress: %d%%\n"), nProgress);
	OutputDebugString(strProgress);
}

// Global flags for update thread management
bool g_bThreadRunning = false;    ///< Indicates if the update check thread is running
bool g_bNewUpdateFound = false;   ///< Set to true if a new update is available

/**
 * @brief Thread procedure for checking for updates.
 *        Runs in a separate thread to avoid blocking the UI.
 * @param lpParam Unused parameter.
 * @return DWORD exit code.
 */
DWORD WINAPI UpdateThreadProc(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    g_bThreadRunning = true;
    // Get the current executable path
    CString strFullPath{ GetModuleFileName() };
    // Check for updates from the remote XML configuration file
    g_bNewUpdateFound = CheckForUpdates(strFullPath.GetString(), _T("https://www.moga.doctor/freeware/genUp4win.xml"), UI_Callback);
    g_bThreadRunning = false;

    ::ExitThread(0);
    // return 0;
}

// Update thread management variables
DWORD m_nUpdateThreadID = 0;   ///< Thread ID of the update checking thread
HANDLE m_hUpdateThread = 0;    ///< Handle to the update checking thread
UINT_PTR m_nTimerID = 0;       ///< Timer ID used to poll thread completion

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
            // Store dialog handle for use in UI_Callback
            hwndDialog = hDlg;
            // Center the update dialog on screen
            CenterWindow(hDlg);
            // Optional: enable progress bar marquee animation
            // SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)30);
            // Create a background thread to check for updates
            m_hUpdateThread = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)UpdateThreadProc, hDlg, 0, &m_nUpdateThreadID);
            // Set up a timer to poll thread completion every 100ms
            m_nTimerID = SetTimer(hDlg, 0x1234, 100, nullptr);
            return (INT_PTR)TRUE;
        case WM_TIMER:
            // Check if the update thread has completed
            if (!g_bThreadRunning)
            {
                // Close the update dialog
                EndDialog(hDlg, IDCANCEL);
                // If a new update was found, exit the application (to allow update installation)
                if (g_bNewUpdateFound)
                {
                    PostQuitMessage(0);
                }
            }
            return 0;
        case WM_COMMAND:
        {
            // Handle manual dialog closure via OK or Cancel buttons
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

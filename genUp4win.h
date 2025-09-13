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

#pragma once
#ifdef __cplusplus

#include <string>
#include <functional>

#ifdef GENUP4WIN_EXPORTS
#define GENUP4WIN __declspec(dllexport)
#else
#define GENUP4WIN __declspec(dllimport)
#endif

/**
 * @brief Status codes used for reporting the state of operations.
 */
typedef enum {
	GENUP4WIN_ERROR = -1,      ///< An error occurred during the operation.
	GENUP4WIN_OK,              ///< The operation completed successfully.
	GENUP4WIN_INPROGRESS       ///< The operation is currently in progress.
} GENUP4WIN_STATUS;

/**
 * @brief Default status callback function. Outputs the status message to the debug output.
 * @param status Status code (see GENUP4WIN_STATUS).
 * @param strMessage Status message.
 */
void StatusCallback(int status, const std::wstring& strMessage) { UNREFERENCED_PARAMETER(status); OutputDebugString(strMessage.c_str()); };

/**
 * @brief Callback function type for reporting status and messages.
 * @param status Status code (see GENUP4WIN_STATUS).
 * @param strMessage Status message.
 */
typedef std::function<void(int, const std::wstring& strMessage)> fnCallback;

/**
 * @brief Generates the full path to the configuration XML file for the application.
 * @param strFilePath The base file path to use if the profile directory is unavailable.
 * @param strProductName The product name, used as the XML file name.
 * @return The full path to the settings XML file.
 */
GENUP4WIN const std::wstring GetAppSettingsFilePath(const std::wstring& strFilePath, const std::wstring& strProductName);

/**
 * @brief Writes configuration data (version and download URL) to an XML file.
 * @param strFilePath Path to the version info file.
 * @param strDownloadURL The download URL to write.
 * @param callback Optional callback function for status/error reporting (default: StatusCallback).
 * @return true if the operation succeeded, false otherwise.
 */
GENUP4WIN bool WriteConfigFile(const std::wstring& strFilePath, const std::wstring& strDownloadURL, fnCallback callback = StatusCallback);

/**
 * @brief Downloads a configuration XML file from a URL, parses it for the latest version and download URL,
 *        and reports status via callback.
 * @param strConfigURL The URL to download the configuration file from.
 * @param strProductName The product name to look up in the XML.
 * @param strLatestVersion Output: receives the latest version string.
 * @param strDownloadURL Output: receives the download URL.
 * @param callback Optional callback function for status/error reporting (default: StatusCallback).
 * @return true if the operation succeeded, false otherwise.
 */
GENUP4WIN bool ReadConfigFile(const std::wstring& strConfigURL, const std::wstring& strProductName, std::wstring& strLatestVersion, std::wstring& strDownloadURL, fnCallback callback = StatusCallback);

/**
 * @brief Checks for software updates by comparing the current version with the latest version from a configuration URL.
 *        If a new version is found, downloads and launches the update, reporting status via callback.
 * @param strFilePath Path to the local version info file.
 * @param strConfigURL URL to the remote configuration XML.
 * @param callback Optional callback function for status/error reporting (default: StatusCallback).
 * @return true if an update was found and download was successful, false otherwise.
 */
GENUP4WIN bool CheckForUpdates(const std::wstring& strFilePath, const std::wstring& strConfigURL, fnCallback callback = StatusCallback);

#endif

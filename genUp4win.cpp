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

#include "pch.h"
#include "resource.h"
#include "genUp4win.h"
#include "AppSettings.h"
#include "VersionInfo.h"

#include "Urlmon.h" // URLDownloadToFile function
#pragma comment(lib, "Urlmon.lib")

#include "shellapi.h" // ShellExecuteEx function
#pragma comment(lib, "Shell32.lib")

#include <comdef.h>
#include <shlobj.h>

/**
 * @brief Constructs the full path to the application's settings XML file.
 *        Tries to use the user's profile directory; falls back to the given file path.
 * @param strFilePath The base file path to use if the profile directory is unavailable.
 * @param strProductName The product name, used as the XML file name.
 * @return The full path to the settings XML file.
 */
const std::wstring GetAppSettingsFilePath(const std::wstring& strFilePath, const std::wstring& strProductName)
{
	WCHAR* lpszSpecialFolderPath = nullptr;
	if ((SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &lpszSpecialFolderPath)) == S_OK)
	{
		std::wstring result(lpszSpecialFolderPath);
		CoTaskMemFree(lpszSpecialFolderPath);
		result += _T("\\");
		result += strProductName;
		result += _T(".xml");
		OutputDebugString(result.c_str());
		return result;
	}

	std::filesystem::path strFullPath{ strFilePath.c_str() };
	strFullPath.replace_filename(strProductName);
	strFullPath.replace_extension(_T(".xml"));
	OutputDebugString(strFullPath.c_str());
	return strFullPath.c_str();
}

/**
 * @brief Writes configuration data (version and download URL) to an XML file.
 * @param strFilePath Path to the version info file.
 * @param strDownloadURL The download URL to write.
 * @param ParentCallback Callback function for status/error reporting.
 * @return true if the operation succeeded, false otherwise.
 */
bool WriteConfigFile(const std::wstring& strFilePath, const std::wstring& strDownloadURL, fnCallback ParentCallback)
{
	bool retVal = false;
	CVersionInfo pVersionInfo;

	if (pVersionInfo.Load(strFilePath.c_str()))
	{
		const std::wstring& strProductName = pVersionInfo.GetProductName();
		try
		{
			const HRESULT hr{ CoInitialize(nullptr) };
			if (FAILED(hr))
			{
				_com_error pError(hr);
				LPCTSTR lpszErrorMessage = pError.ErrorMessage();
				ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
				return false;
			}

			// Write version and download URL to XML settings
			CXMLAppSettings pAppSettings(GetAppSettingsFilePath(strFilePath, strProductName), true, true);
			pAppSettings.WriteString(strProductName.c_str(), VERSION_ENTRY_ID, pVersionInfo.GetProductVersionAsString().c_str());
			pAppSettings.WriteString(strProductName.c_str(), DOWNLOAD_ENTRY_ID, strDownloadURL.c_str());
			retVal = true;
		}
		catch (CAppSettingsException& pException)
		{
			const int nErrorLength = 0x100;
			TCHAR lpszErrorMessage[nErrorLength] = { 0, };
			pException.GetErrorMessage(lpszErrorMessage, nErrorLength);
			ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
		}
	}
	return retVal;
}

/**
 * @brief Downloads a configuration XML file from a URL, parses it for the latest version and download URL,
 *        and reports status via callback.
 * @param strConfigURL The URL to download the configuration file from.
 * @param strProductName The product name to look up in the XML.
 * @param strLatestVersion Output: receives the latest version string.
 * @param strDownloadURL Output: receives the download URL.
 * @param ParentCallback Callback function for status/error reporting.
 * @return true if the operation succeeded, false otherwise.
 */
bool ReadConfigFile(const std::wstring& strConfigURL, const std::wstring& strProductName, std::wstring& strLatestVersion, std::wstring& strDownloadURL, fnCallback ParentCallback)
{
	CString strStatusMessage;
	HRESULT hResult = S_OK;
	bool retVal = false;
	TCHAR lpszTempPath[_MAX_PATH + 1] = { 0, };
	DWORD nLength = GetTempPath(_MAX_PATH, lpszTempPath);
	if (nLength > 0)
	{
		TCHAR lpszFilePath[_MAX_PATH + 1] = { 0, };
		nLength = GetTempFileName(lpszTempPath, L"GUP", 0, lpszFilePath);
		if (nLength > 0)
		{
			CString strFileName = lpszFilePath;
			strFileName.Replace(_T(".tmp"), _T(".xml"));
			if (strStatusMessage.LoadString(IDS_CONNECTING))
			{
				ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
			}
			if ((hResult = URLDownloadToFile(nullptr, strConfigURL.c_str(), strFileName, 0, nullptr)) == S_OK)
			{
				try
				{
					const HRESULT hr{ CoInitialize(nullptr) };
					if (FAILED(hr))
					{
						_com_error pError(hr);
						LPCTSTR lpszErrorMessage = pError.ErrorMessage();
						ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
						return false;
					}

					// Parse XML for version and download URL
					CXMLAppSettings pAppSettings(std::wstring(strFileName), true, true);
					strLatestVersion = pAppSettings.GetString(strProductName.c_str(), VERSION_ENTRY_ID);
					strDownloadURL = pAppSettings.GetString(strProductName.c_str(), DOWNLOAD_ENTRY_ID);
					retVal = true;
				}
				catch (CAppSettingsException& pException)
				{
					const int nErrorLength = 0x100;
					TCHAR lpszErrorMessage[nErrorLength] = { 0, };
					pException.GetErrorMessage(lpszErrorMessage, nErrorLength);
					ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
				}
			}
			else
			{
				_com_error pError(hResult);
				LPCTSTR lpszErrorMessage = pError.ErrorMessage();
				ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
			}
		}
	}
	return retVal;
}

/**
 * @brief Checks for software updates by comparing the current version with the latest version from a configuration URL.
 *        If a new version is found, downloads and launches the update, reporting status via callback.
 * @param strFilePath Path to the local version info file.
 * @param strConfigURL URL to the remote configuration XML.
 * @param ParentCallback Callback function for status/error reporting.
 * @return true if an update was found and download was successful, false otherwise.
 */
bool CheckForUpdates(const std::wstring& strFilePath, const std::wstring& strConfigURL, fnCallback ParentCallback)
{
	CString strStatusMessage;
	HRESULT hResult = S_OK;
	bool retVal = false;
	CVersionInfo pVersionInfo;
	std::wstring strLatestVersion, strDownloadURL;

	if (pVersionInfo.Load(strFilePath.c_str()))
	{
		const std::wstring& strProductName = pVersionInfo.GetProductName();
		OutputDebugString(strProductName.c_str());
		if (ReadConfigFile(strConfigURL, strProductName, strLatestVersion, strDownloadURL, ParentCallback))
		{
			const bool bNewUpdateFound = (strLatestVersion.compare(pVersionInfo.GetProductVersionAsString()) != 0);
			if (bNewUpdateFound)
			{
				TCHAR lpszTempPath[_MAX_PATH + 1] = { 0, };
				DWORD nLength = GetTempPath(_MAX_PATH, lpszTempPath);
				if (nLength > 0)
				{
					TCHAR lpszFilePath[_MAX_PATH + 1] = { 0, };
					nLength = GetTempFileName(lpszTempPath, L"GUP", 0, lpszFilePath);
					if (nLength > 0)
					{
						CString strFileName = lpszFilePath;
						strFileName.Replace(_T(".tmp"), DEFAULT_EXTENSION);
						if (strStatusMessage.LoadString(IDS_DOWNLOADING))
						{
							ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
						}
						if ((hResult = URLDownloadToFile(nullptr, strDownloadURL.c_str(), strFileName, 0, nullptr)) == S_OK)
						{
							// Launch the downloaded update
							SHELLEXECUTEINFO pShellExecuteInfo;
							pShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
							pShellExecuteInfo.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_DOENVSUBST;
							pShellExecuteInfo.hwnd = nullptr;
							pShellExecuteInfo.lpVerb = _T("open");
							pShellExecuteInfo.lpFile = strFileName;
							pShellExecuteInfo.lpParameters = nullptr;
							pShellExecuteInfo.lpDirectory = nullptr;
							pShellExecuteInfo.nShow = SW_SHOWNORMAL;
							const bool bLauched = ShellExecuteEx(&pShellExecuteInfo);
							if (strStatusMessage.LoadString(bLauched ? IDS_SUCCESS : IDS_FAILED))
							{
								ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
							}
							retVal = true; // Download was successful
						}
						else
						{
							_com_error pError(hResult);
							LPCTSTR lpszErrorMessage = pError.ErrorMessage();
							ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
						}
					}
				}
			}
		}
	}
	return retVal;
}

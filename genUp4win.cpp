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
#include "SHA256.h"

/**
 * @brief Converts a UTF-8 encoded string to a wide string (UTF-16).
 * @param string The UTF-8 string to convert.
 * @return The converted wide string.
 * @throws std::runtime_error if conversion fails.
 */
std::wstring utf8_to_wstring(const std::string& string)
{
	// Handle empty string case early
	if (string.empty())
	{
		return L"";
	}

	// Calculate the required buffer size for the wide string
	// Use MB_ERR_INVALID_CHARS to fail on invalid UTF-8 sequences
	const int size_needed = MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		string.c_str(),
		static_cast<int>(string.length()),
		nullptr,
		0
	);

	if (size_needed == 0)
	{
		const DWORD error = GetLastError();
		throw std::runtime_error("MultiByteToWideChar() failed with error code: " + std::to_string(error));
	}

	// Allocate buffer and perform the conversion
	std::wstring result(size_needed, L'\0');
	const int converted = MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		string.c_str(),
		static_cast<int>(string.length()),
		result.data(),
		size_needed
	);

	if (converted == 0)
	{
		const DWORD error = GetLastError();
		throw std::runtime_error("MultiByteToWideChar() conversion failed with error code: " + std::to_string(error));
	}

	return result;
}

std::string wstring_to_utf8(const std::wstring& wide_string)
{
	// Handle empty string case early
	if (wide_string.empty())
	{
		return "";
	}

	// Calculate the required buffer size for the UTF-8 string
	// Use WC_ERR_INVALID_CHARS to fail on invalid UTF-16 sequences
	const int size_needed = WideCharToMultiByte(
		CP_UTF8,
		WC_ERR_INVALID_CHARS,
		wide_string.c_str(),
		static_cast<int>(wide_string.length()),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (size_needed == 0)
	{
		const DWORD error = GetLastError();
		throw std::runtime_error("WideCharToMultiByte() failed with error code: " + std::to_string(error));
	}

	// Allocate buffer and perform the conversion
	std::string result(size_needed, '\0');
	const int converted = WideCharToMultiByte(
		CP_UTF8,
		WC_ERR_INVALID_CHARS,
		wide_string.c_str(),
		static_cast<int>(wide_string.length()),
		result.data(),
		size_needed,
		nullptr,
		nullptr
	);

	if (converted == 0)
	{
		const DWORD error = GetLastError();
		throw std::runtime_error("WideCharToMultiByte() conversion failed with error code: " + std::to_string(error));
	}

	return result;
}

const int MAX_BUFFER = 0x10000; ///< Maximum buffer size for file operations.

/**
 * @brief Calculates the SHA256 checksum of a file.
 * @param strFilePath Path to the file to calculate checksum for.
 * @param strChecksum Output: receives the calculated checksum as a hexadecimal string.
 * @return true if the checksum was calculated successfully, false if the file couldn't be opened.
 */
bool GetChecksumFromFile(const std::wstring& strFilePath, std::wstring& strChecksum)
{
	SHA256 sha256;

	// Open the file in binary mode
	std::ifstream file(strFilePath, std::ios::binary);
	if (!file)
	{
		return false;
	}

	// Use a dynamically allocated buffer to avoid stack overflow
	constexpr size_t bufferSize = MAX_BUFFER;
	std::vector<char> buffer(bufferSize);

	// Read the file in chunks and update the SHA256 hash
	while (file.read(buffer.data(), bufferSize) || file.gcount() > 0)
	{
		sha256.update(reinterpret_cast<const uint8_t*>(buffer.data()),
			static_cast<size_t>(file.gcount()));
	}

	// Convert the digest to a hexadecimal string
	strChecksum = utf8_to_wstring(SHA256::toString(sha256.digest()));
	return true;
}

/**
 * @brief Downloads a file from a URL and calculates its SHA256 checksum.
 * @param strURL The URL to download the file from.
 * @param strChecksum Output: receives the calculated checksum as a hexadecimal string.
 * @return true if the download and checksum calculation succeeded, false otherwise.
 */
bool GetChecksumFromURL(const std::wstring strURL, std::wstring& strChecksum)
{
	HRESULT hResult = S_OK;
	TCHAR lpszTempPath[_MAX_PATH + 1] = { 0, };

	// Get the system's temporary directory path
	DWORD nLength = GetTempPath(_MAX_PATH, lpszTempPath);
	if (nLength > 0)
	{
		TCHAR lpszFilePath[_MAX_PATH + 1] = { 0, };

		// Generate a unique temporary file name
		nLength = GetTempFileName(lpszTempPath, L"GUP", 0, lpszFilePath);
		if (nLength > 0)
		{
			// Change the extension from .tmp to .msi (or the default installer extension)
			CString strFileName = lpszFilePath;
			strFileName.Replace(_T(".tmp"), DEFAULT_EXTENSION);

			// Download the file from the URL
			if ((hResult = URLDownloadToFile(nullptr, strURL.c_str(), strFileName, 0, nullptr)) == S_OK)
			{
				// Calculate and return the checksum of the downloaded file
				return GetChecksumFromFile(strFileName.GetString(), strChecksum);
			}
		}
	}

	return false;
}

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

	// Attempt to get the user's profile directory (e.g., C:\Users\Username)
	if ((SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &lpszSpecialFolderPath)) == S_OK)
	{
		// Build the settings file path in the user's profile directory
		std::wstring result(lpszSpecialFolderPath);
		CoTaskMemFree(lpszSpecialFolderPath); // Free the allocated memory
		result += _T("\\");
		result += strProductName;
		result += _T(".xml");
		OutputDebugString(result.c_str()); // Log the path for debugging
		return result;
	}

	// Fallback: Use the application's directory if profile directory is unavailable
	std::filesystem::path strFullPath{ strFilePath.c_str() };
	strFullPath.replace_filename(strProductName);
	strFullPath.replace_extension(_T(".xml"));
	OutputDebugString(strFullPath.c_str()); // Log the path for debugging
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
	std::wstring strChecksum; // Stores the calculated checksum of the download URL

	// Load version information from the specified file
	if (pVersionInfo.Load(strFilePath.c_str()))
	{
		const std::wstring& strProductName = pVersionInfo.GetProductName();
		try
		{
			// Initialize COM library for XML operations
			const HRESULT hr{ CoInitialize(nullptr) };
			if (FAILED(hr))
			{
				// Report COM initialization failure
				_com_error pError(hr);
				LPCTSTR lpszErrorMessage = pError.ErrorMessage();
				ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
				return false;
			}

			// Write version and download URL to XML settings file
			CXMLAppSettings pAppSettings(GetAppSettingsFilePath(strFilePath, strProductName), true, true);
			pAppSettings.WriteString(strProductName.c_str(), VERSION_ENTRY_ID, pVersionInfo.GetProductVersionAsString().c_str());
			pAppSettings.WriteString(strProductName.c_str(), DOWNLOAD_ENTRY_ID, strDownloadURL.c_str());

			// Calculate and write the checksum of the download URL if available
			if (GetChecksumFromURL(strDownloadURL.c_str(), strChecksum))
			{
				pAppSettings.WriteString(strProductName.c_str(), CHECKSUM_ENTRY_ID, strChecksum.c_str());
			}
			retVal = true;
		}
		catch (CAppSettingsException& pException)
		{
			// Handle XML settings exceptions
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
 * @param strChecksum Output: receives the checksum string.
 * @param ParentCallback Callback function for status/error reporting.
 * @return true if the operation succeeded, false otherwise.
 */
bool ReadConfigFile(const std::wstring& strConfigURL, const std::wstring& strProductName, std::wstring& strLatestVersion, std::wstring& strDownloadURL, std::wstring& strChecksum, fnCallback ParentCallback)
{
	CString strStatusMessage;
	HRESULT hResult = S_OK;
	bool retVal = false;
	TCHAR lpszTempPath[_MAX_PATH + 1] = { 0, };

	// Get the system's temporary directory path
	DWORD nLength = GetTempPath(_MAX_PATH, lpszTempPath);
	if (nLength > 0)
	{
		TCHAR lpszFilePath[_MAX_PATH + 1] = { 0, };

		// Generate a unique temporary file name
		nLength = GetTempFileName(lpszTempPath, L"GUP", 0, lpszFilePath);
		if (nLength > 0)
		{
			// Change the extension from .tmp to .xml
			CString strFileName = lpszFilePath;
			strFileName.Replace(_T(".tmp"), _T(".xml"));

			// Report connection status to the user
			if (strStatusMessage.LoadString(IDS_CONNECTING))
			{
				ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
			}

			// Download the configuration file from the URL
			if ((hResult = URLDownloadToFile(nullptr, strConfigURL.c_str(), strFileName, 0, nullptr)) == S_OK)
			{
				try
				{
					// Initialize COM library for XML operations
					const HRESULT hr{ CoInitialize(nullptr) };
					if (FAILED(hr))
					{
						// Report COM initialization failure
						_com_error pError(hr);
						LPCTSTR lpszErrorMessage = pError.ErrorMessage();
						ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
						return false;
					}

					// Parse XML configuration file for version, download URL, and checksum
					CXMLAppSettings pAppSettings(std::wstring(strFileName), true, true);
					strLatestVersion = pAppSettings.GetString(strProductName.c_str(), VERSION_ENTRY_ID);
					strDownloadURL = pAppSettings.GetString(strProductName.c_str(), DOWNLOAD_ENTRY_ID);
					strChecksum = pAppSettings.GetString(strProductName.c_str(), CHECKSUM_ENTRY_ID);
					retVal = true;
				}
				catch (CAppSettingsException& pException)
				{
					// Handle XML parsing exceptions
					const int nErrorLength = 0x100;
					TCHAR lpszErrorMessage[nErrorLength] = { 0, };
					pException.GetErrorMessage(lpszErrorMessage, nErrorLength);
					ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage);
				}
			}
			else
			{
				// Report download failure
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
	std::wstring strLatestVersion,
		strDownloadURL,
		strChecksum;

	// Load the current application's version information
	if (pVersionInfo.Load(strFilePath.c_str()))
	{
		const std::wstring& strProductName = pVersionInfo.GetProductName();
		OutputDebugString(strProductName.c_str()); // Log product name for debugging

		// Download and parse the remote configuration file
		if (ReadConfigFile(strConfigURL, strProductName, strLatestVersion, strDownloadURL, strChecksum, ParentCallback))
		{
			// Compare current version with the latest version from the server
			const bool bNewUpdateFound = (strLatestVersion.compare(pVersionInfo.GetProductVersionAsString()) != 0);
			if (bNewUpdateFound)
			{
				TCHAR lpszTempPath[_MAX_PATH + 1] = { 0, };

				// Get the system's temporary directory path
				DWORD nLength = GetTempPath(_MAX_PATH, lpszTempPath);
				if (nLength > 0)
				{
					TCHAR lpszFilePath[_MAX_PATH + 1] = { 0, };

					// Generate a unique temporary file name
					nLength = GetTempFileName(lpszTempPath, L"GUP", 0, lpszFilePath);
					if (nLength > 0)
					{
						// Change the extension from .tmp to the default installer extension
						CString strFileName = lpszFilePath;
						strFileName.Replace(_T(".tmp"), DEFAULT_EXTENSION);

						// Report download status to the user
						if (strStatusMessage.LoadString(IDS_DOWNLOADING))
						{
							ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
						}

						// Download the update installer
						if ((hResult = URLDownloadToFile(nullptr, strDownloadURL.c_str(), strFileName, 0, nullptr)) == S_OK)
						{
							// Verify the downloaded file's checksum if available
							if (!strChecksum.empty())
							{
								std::wstring strDownloadedFileChecksum;

								// Calculate the checksum of the downloaded file
								if (GetChecksumFromFile(strFileName.GetString(), strDownloadedFileChecksum))
								{
									// Compare with the expected checksum
									if (strDownloadedFileChecksum.compare(strChecksum) != 0)
									{
										// Checksum mismatch - report error
										if (strStatusMessage.LoadString(IDS_CHECKSUM_MISMATCH))
										{
											ParentCallback(GENUP4WIN_ERROR, std::wstring(strStatusMessage));
										}
										::MessageBeep(MB_ICONERROR); // Alert the user with a beep
										return false; // Checksum mismatch, do not proceed with the update
									}
								}
								else
								{
									// Failed to calculate checksum - report error
									if (strStatusMessage.LoadString(IDS_CHECKSUM_CALCULATION_FAILED))
									{
										ParentCallback(GENUP4WIN_ERROR, std::wstring(strStatusMessage));
									}
									return false; // Failed to calculate checksum, do not proceed with the update
								}
							}
							::MessageBeep(MB_OK); // Alert the user with a beep that the download is complete

							// Launch the downloaded update installer
							SHELLEXECUTEINFO pShellExecuteInfo;
							pShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
							pShellExecuteInfo.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_DOENVSUBST;
							pShellExecuteInfo.hwnd = nullptr;
							pShellExecuteInfo.lpVerb = _T("open");
							pShellExecuteInfo.lpFile = strFileName;
							pShellExecuteInfo.lpParameters = nullptr;
							pShellExecuteInfo.lpDirectory = nullptr;
							pShellExecuteInfo.nShow = SW_SHOWNORMAL;

							// Execute the installer
							const bool bLauched = ShellExecuteEx(&pShellExecuteInfo);

							// Report success or failure status
							if (strStatusMessage.LoadString(bLauched ? IDS_SUCCESS : IDS_FAILED))
							{
								ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage));
							}
							retVal = true; // Download was successful
						}
						else
						{
							// Report download failure
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

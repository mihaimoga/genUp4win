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
 * @brief IBindStatusCallback implementation for URLDownloadToFile to report download progress.
 * 
 * This class implements the COM IBindStatusCallback interface to provide progress notifications
 * during file downloads using URLDownloadToFile. It tracks download progress and reports
 * percentage completion to a user-provided callback function.
 * 
 * @note This class manages its own lifetime through COM reference counting.
 *       It will automatically delete itself when the reference count reaches zero.
 */
class CDownloadCallback : public IBindStatusCallback
{
private:
	ULONG m_refCount;           ///< COM reference count for object lifetime management
	fnCallback m_callback;      ///< User callback function for progress notifications
	ULONGLONG m_totalBytes;     ///< Total size of the file being downloaded in bytes
	ULONGLONG m_downloadedBytes; ///< Number of bytes downloaded so far

public:
	/**
	 * @brief Constructor that initializes the callback with a user-provided function.
	 * @param callback The callback function to be invoked with progress updates.
	 */
	CDownloadCallback(fnCallback callback) 
		: m_refCount(1), m_callback(callback), m_totalBytes(0), m_downloadedBytes(0)
	{
	}

	// ========================================================================================
	// IUnknown methods - Required for all COM interfaces
	// ========================================================================================

	/**
	 * @brief Queries whether this object supports a specific COM interface.
	 * @param riid The interface identifier (IID) being queried.
	 * @param ppvObject Output pointer to receive the interface pointer if supported.
	 * @return S_OK if the interface is supported, E_NOINTERFACE otherwise.
	 */
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		// Check if the requested interface is IUnknown or IBindStatusCallback
		if (riid == IID_IUnknown || riid == IID_IBindStatusCallback)
		{
			*ppvObject = static_cast<IBindStatusCallback*>(this);
			AddRef(); // Increment reference count for the returned interface
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE; // Interface not supported
	}

	/**
	 * @brief Increments the COM reference count for this object.
	 * @return The new reference count after incrementing.
	 */
	STDMETHOD_(ULONG, AddRef)()
	{
		// Use thread-safe increment for COM reference counting
		return InterlockedIncrement(&m_refCount);
	}

	/**
	 * @brief Decrements the COM reference count and deletes the object if count reaches zero.
	 * @return The new reference count after decrementing.
	 */
	STDMETHOD_(ULONG, Release)()
	{
		// Use thread-safe decrement for COM reference counting
		ULONG count = InterlockedDecrement(&m_refCount);
		if (count == 0)
		{
			// No more references - delete this object
			delete this;
		}
		return count;
	}

	// ========================================================================================
	// IBindStatusCallback methods - Provide download status notifications
	// ========================================================================================

	/**
	 * @brief Called when the bind operation starts.
	 * @param dwReserved Reserved for future use.
	 * @param pib Pointer to the IBinding interface for the bind operation.
	 * @return S_OK to continue the operation.
	 */
	STDMETHOD(OnStartBinding)(DWORD, IBinding*)
	{
		return S_OK; // Continue with the bind operation
	}

	/**
	 * @brief Called to get the priority of the bind operation.
	 * @param pnPriority Pointer to receive the priority value.
	 * @return E_NOTIMPL as we don't implement custom priority handling.
	 */
	STDMETHOD(GetPriority)(LONG*)
	{
		return E_NOTIMPL; // We don't implement custom priority
	}

	/**
	 * @brief Called when the system is running low on resources.
	 * @param reserved Reserved for future use.
	 * @return S_OK to continue the operation.
	 */
	STDMETHOD(OnLowResource)(DWORD)
	{
		return S_OK; // Continue despite low resources
	}

	/**
	 * @brief Called periodically to report download progress.
	 * 
	 * This is the main method that tracks and reports download progress. It calculates
	 * the percentage complete and notifies the user callback function.
	 * 
	 * @param ulProgress Number of bytes downloaded so far.
	 * @param ulProgressMax Total size of the download in bytes.
	 * @param ulStatusCode Status code indicating the type of progress notification.
	 * @param szStatusText Optional status text (not used in this implementation).
	 * @return S_OK to continue the operation.
	 */
	STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR)
	{
		// Only process download progress notifications
		// BINDSTATUS_DOWNLOADINGDATA: Called periodically during download
		// BINDSTATUS_ENDDOWNLOADDATA: Called when download completes
		if (ulStatusCode == BINDSTATUS_DOWNLOADINGDATA || ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
		{
			// Ensure we have valid progress data to avoid division by zero
			if (ulProgressMax > 0)
			{
				// Update our internal progress tracking
				m_downloadedBytes = ulProgress;
				m_totalBytes = ulProgressMax;

				// Calculate percentage complete (0-100)
				const int percentage = static_cast<int>((ulProgress * 100) / ulProgressMax);

				// Notify the user callback if it's set
				if (m_callback)
				{
					CString strStatusMessage;
					// Load the localized "Downloading..." message from resources
					if (strStatusMessage.LoadString(IDS_DOWNLOADING))
					{
						// Report progress to the parent application with current percentage
						m_callback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage), percentage);
					}
				}
			}
		}
		return S_OK; // Continue the download
	}

	/**
	 * @brief Called when the bind operation stops.
	 * @param hresult The final result code of the bind operation.
	 * @param szError Optional error description (not used in this implementation).
	 * @return S_OK.
	 */
	STDMETHOD(OnStopBinding)(HRESULT, LPCWSTR)
	{
		return S_OK; // Bind operation has completed
	}

	/**
	 * @brief Called to get bind information for the operation.
	 * @param grfBINDF Pointer to receive bind flags.
	 * @param pbindinfo Pointer to BINDINFO structure to fill.
	 * @return S_OK with default bind settings.
	 */
	STDMETHOD(GetBindInfo)(DWORD*, BINDINFO*)
	{
		return S_OK; // Use default bind settings
	}

	/**
	 * @brief Called when downloaded data becomes available.
	 * @param grfBSCF Flags indicating the type of data available.
	 * @param dwSize Size of the available data.
	 * @param pformatetc Pointer to FORMATETC structure describing the data format.
	 * @param pstgmed Pointer to STGMEDIUM structure containing the data.
	 * @return S_OK.
	 */
	STDMETHOD(OnDataAvailable)(DWORD, DWORD, FORMATETC*, STGMEDIUM*)
	{
		return S_OK; // Data is being handled by URLDownloadToFile
	}

	/**
	 * @brief Called when a bound object is available.
	 * @param riid The interface identifier of the object.
	 * @param punk Pointer to the IUnknown interface of the object.
	 * @return S_OK.
	 */
	STDMETHOD(OnObjectAvailable)(REFIID, IUnknown*)
	{
		return S_OK; // Object availability notification received
	}
};

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
				ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
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
			ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
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
				ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage), 0);
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
						ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
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
					ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
				}
			}
			else
			{
				// Report download failure
				_com_error pError(hResult);
				LPCTSTR lpszErrorMessage = pError.ErrorMessage();
				ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
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

						// Report initial download status to the user (0% progress)
						if (strStatusMessage.LoadString(IDS_DOWNLOADING))
						{
							ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage), 0);
						}

						// Create progress callback handler and download the update installer
						// CDownloadCallback will receive progress notifications from URLDownloadToFile
						CDownloadCallback pCallback(ParentCallback);
						if ((hResult = URLDownloadToFile(nullptr, strDownloadURL.c_str(), strFileName, 0, &pCallback)) == S_OK)
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
											ParentCallback(GENUP4WIN_ERROR, std::wstring(strStatusMessage), 0);
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
										ParentCallback(GENUP4WIN_ERROR, std::wstring(strStatusMessage), 0);
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
								ParentCallback(GENUP4WIN_INPROGRESS, std::wstring(strStatusMessage), 0);
							}
							retVal = true; // Download was successful
						}
						else
						{
							// Report download failure
							_com_error pError(hResult);
							LPCTSTR lpszErrorMessage = pError.ErrorMessage();
							ParentCallback(GENUP4WIN_ERROR, lpszErrorMessage, 0);
						}
					}
				}
			}
		}
	}
	return retVal;
}

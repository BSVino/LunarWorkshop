#include <tinker_platform.h>

#include <windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <dbghelp.h>

#include <tstring.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses)
{
	static unsigned char aiAddresses[16][8];

	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(
		AdapterInfo,
		&dwBufLen);

	iAddresses = 0;

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	do {
		// Use only ethernet, not wireless controllers. They are more reliable and wireless controllers get disabled sometimes.
		if (pAdapterInfo->Type != MIB_IF_TYPE_ETHERNET)
			continue;

		// Skip virtual controllers, they can be changed or disabled.
		if (strstr(pAdapterInfo->Description, "VMware"))
			continue;

		if (strstr(pAdapterInfo->Description, "Hamachi"))
			continue;

		// Skip USB controllers, they can be unplugged.
		if (strstr(pAdapterInfo->Description, "USB"))
			continue;

		memcpy(aiAddresses[iAddresses++], pAdapterInfo->Address, sizeof(char)*8);
	}
	while(pAdapterInfo = pAdapterInfo->Next);

	paiAddresses = &aiAddresses[0][0];
}

void GetScreenSize(int& iWidth, int& iHeight)
{
	iWidth = GetSystemMetrics(SM_CXSCREEN);
	iHeight = GetSystemMetrics(SM_CYSCREEN);
}

size_t GetNumberOfProcessors()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
}

void SleepMS(size_t iMS)
{
	Sleep(iMS);
}

void OpenBrowser(const tstring& sURL)
{
	ShellExecute(NULL, L"open", convertstring<tchar, wchar_t>(sURL).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void OpenExplorer(const tstring& sDirectory)
{
	ShellExecute(NULL, L"open", convertstring<tchar, wchar_t>(sDirectory).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static int g_iMinidumpsWritten = 0;

void CreateMinidump(void* pInfo, tchar* pszDirectory)
{
#ifndef _DEBUG
	time_t currTime = ::time( NULL );
	struct tm * pTime = ::localtime( &currTime );

	wchar_t szModule[MAX_PATH];
	::GetModuleFileName( NULL, szModule, sizeof(szModule) / sizeof(tchar) );
	wchar_t *pModule = wcsrchr( szModule, '.' );

	if ( pModule )
		*pModule = 0;

	pModule = wcsrchr( szModule, '\\' );
	if ( pModule )
		pModule++;
	else
		pModule = L"unknown";

	wchar_t szFileName[MAX_PATH];
	_snwprintf( szFileName, sizeof(szFileName) / sizeof(tchar),
			L"%s_%d.%.2d.%2d.%.2d.%.2d.%.2d_%d.mdmp",
			pModule,
			pTime->tm_year + 1900,
			pTime->tm_mon + 1,
			pTime->tm_mday,
			pTime->tm_hour,
			pTime->tm_min,
			pTime->tm_sec,
			g_iMinidumpsWritten++
			);

	HANDLE hFile = CreateFile( convertstring<tchar, wchar_t>(GetAppDataDirectory(pszDirectory, convertstring<wchar_t, tchar>(szFileName))).c_str(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) )
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId           = GetCurrentThreadId();
		mdei.ExceptionPointers  = (EXCEPTION_POINTERS*)pInfo;
		mdei.ClientPointers     = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine     = NULL;
		mci.CallbackParam       = 0;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pInfo != 0) ? &mdei : 0, 0, &mci );

		if( rv )
		{
			// Success... message to user?
		}

		CloseHandle( hFile );
	}
#endif
}

eastl::string GetClipboard()
{
	if (!OpenClipboard(NULL))
		return "";

	HANDLE hData = GetClipboardData(CF_TEXT);
	char* szBuffer = (char*)GlobalLock(hData);
	GlobalUnlock(hData);
	CloseClipboard();

	eastl::string sClipboard(szBuffer);

	return sClipboard;
}

void SetClipboard(const eastl::string& sBuf)
{
	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL hClipboard;
	hClipboard = GlobalAlloc(GMEM_MOVEABLE, sBuf.length()+1);

	char* pszBuffer = (char*)GlobalLock(hClipboard);
	strcpy(pszBuffer, LPCSTR(sBuf.c_str()));

	GlobalUnlock(hClipboard);

	SetClipboardData(CF_TEXT, hClipboard);

	CloseClipboard();
}

tstring GetAppDataDirectory(const tstring& sDirectory, const tstring& sFile)
{
	size_t iSize;
	_wgetenv_s(&iSize, NULL, 0, L"APPDATA");

	tstring sSuffix;
	sSuffix.append(sDirectory).append("\\").append(sFile);

	if (!iSize)
		return sSuffix;

	wchar_t* pszVar = (wchar_t*)malloc(iSize * sizeof(wchar_t));
	if (!pszVar)
		return sSuffix;

	_wgetenv_s(&iSize, pszVar, iSize, L"APPDATA");

	tstring sReturn = convertstring<wchar_t, tchar>(pszVar);

	free(pszVar);

	CreateDirectory(convertstring<tchar, wchar_t>(tstring(sReturn).append("\\").append(sDirectory)).c_str(), NULL);

	sReturn.append("\\").append(sSuffix);
	return sReturn;
}

eastl::vector<tstring> ListDirectory(tstring sDirectory, bool bDirectories)
{
	eastl::vector<tstring> asResult;

	wchar_t szPath[MAX_PATH];
	_swprintf(szPath, L"%s\\*", convertstring<tchar, wchar_t>(sDirectory).c_str());

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szPath, &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			if (!bDirectories && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// Duh.
			if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
				continue;

			asResult.push_back(convertstring<wchar_t, tchar>(fd.cFileName));
		} while(FindNextFile(hFind, &fd));

		FindClose(hFind);
	}

	return asResult;
}

bool IsFile(tstring sPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(convertstring<tchar, wchar_t>(sPath).c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;

	return true;
}

bool IsDirectory(tstring sPath)
{
	while (sPath.substr(sPath.length()-1) == DIR_SEP)
		sPath = sPath.substr(0, sPath.length()-1);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(convertstring<tchar, wchar_t>(sPath).c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

tstring FindAbsolutePath(const tstring& sPath)
{
	wchar_t szPath[MAX_PATH];
	eastl::wstring swPath = convertstring<tchar, wchar_t>(sPath);

	GetFullPathName(swPath.c_str(), MAX_PATH, szPath, nullptr);

	return convertstring<wchar_t, tchar>(szPath);
}

void DebugPrint(tstring sText)
{
	OutputDebugString(convertstring<tchar, wchar_t>(sText).c_str());
}

void Exec(eastl::string sLine)
{
	system(sLine.c_str());
}

int GetVKForChar(int iChar)
{
	switch(iChar)
	{
	case ';':
		return VK_OEM_1;

	case '/':
		return VK_OEM_2;

	case '`':
		return VK_OEM_3;

	case '[':
		return VK_OEM_4;

	case '\\':
		return VK_OEM_5;

	case ']':
		return VK_OEM_6;

	case '\'':
		return VK_OEM_7;

	case '=':
		return VK_OEM_PLUS;

	case ',':
		return VK_OEM_COMMA;

	case '-':
		return VK_OEM_MINUS;

	case '.':
		return VK_OEM_PERIOD;
	}

	return iChar;
}

int TranslateKeyToQwerty(int iKey)
{
	// If we are using a non-qwerty layout, map the keys to querty internally.

	HKL iCurrent = GetKeyboardLayout( 0 );
	static HKL iEnglish = LoadKeyboardLayout(L"00000409", 0);

	if (iCurrent == iEnglish)
		return iKey;

	UINT i = MapVirtualKeyEx(GetVKForChar(iKey), MAPVK_VK_TO_VSC, iCurrent);
	return (int)MapVirtualKeyEx(i, MAPVK_VSC_TO_VK, iEnglish);
}

int TranslateKeyFromQwerty(int iKey)
{
	// If we are using a non-qwerty layout, map the keys to querty internally.

	HKL iCurrent = GetKeyboardLayout( 0 );
	static HKL iEnglish = LoadKeyboardLayout(L"00000409", 0);

	if (iCurrent == iEnglish)
		return iKey;

	UINT i = MapVirtualKeyEx(GetVKForChar(iKey), MAPVK_VK_TO_VSC, iEnglish);
	return (int)MapVirtualKeyEx(i, MAPVK_VSC_TO_VK, iCurrent);
}

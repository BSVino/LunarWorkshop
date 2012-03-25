#ifndef _LW_PLATFORM
#define _LW_PLATFORM

#include <EASTL/vector.h>

#include <tstring.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
void GetScreenSize(int& iWidth, int& iHeight);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);
void OpenBrowser(const tstring& sURL);
void OpenExplorer(const tstring& sDirectory);
void Alert(const tstring& sMessage);
void CreateMinidump(void* pInfo, tchar* pszDirectory);
eastl::string GetClipboard();
void SetClipboard(const eastl::string& sBuf);
tstring GetAppDataDirectory(const tstring& sDirectory, const tstring& sFile = "");
eastl::vector<tstring> ListDirectory(const tstring& sDirectory, bool bDirectories = true);
bool IsFile(const tstring& sPath);
bool IsDirectory(const tstring& sPath);
void CreateDirectoryNonRecursive(const tstring& sPath);
bool CopyFileTo(const tstring& sFrom, const tstring& sTo, bool bOverride = true);
tstring FindAbsolutePath(const tstring& sPath);
time_t GetFileModificationTime(const char* pszFile);
void DebugPrint(const tstring& sText);
void Exec(const eastl::string& sLine);
int TranslateKeyToQwerty(int iKey);
int TranslateKeyFromQwerty(int iKey);

#ifdef _WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

#endif

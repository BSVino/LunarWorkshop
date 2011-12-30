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
void CreateMinidump(void* pInfo, tchar* pszDirectory);
eastl::string GetClipboard();
void SetClipboard(const eastl::string& sBuf);
tstring GetAppDataDirectory(const tstring& sDirectory, const tstring& sFile);
eastl::vector<tstring> ListDirectory(tstring sDirectory, bool bDirectories = true);
bool IsFile(tstring sPath);
bool IsDirectory(tstring sPath);
tstring FindAbsolutePath(const tstring& sPath);
time_t GetFileModificationTime(const char* pszFile);
void DebugPrint(tstring sText);
void Exec(eastl::string sLine);
int TranslateKeyToQwerty(int iKey);
int TranslateKeyFromQwerty(int iKey);

#ifdef _WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

#endif

#include <tinker_platform.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <X11/Xlib.h>

#include <strutils.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses)
{
	static unsigned char aiAddresses[16][8];

	struct ifreq ifr;
	struct ifreq *IFR;
	struct ifconf ifc;
   	char buf[1024];
  	int s, i;

	iAddresses = 0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
   	if (s == -1)
  		return;

	ifc.ifc_len = sizeof(buf);
   	ifc.ifc_buf = buf;
  	ioctl(s, SIOCGIFCONF, &ifc);

 	IFR = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++)
	{
		if (iAddresses >= 16)
			break;

   		strcpy(ifr.ifr_name, IFR->ifr_name);
  		if (ioctl(s, SIOCGIFFLAGS, &ifr) != 0)
			continue;
 
		if (ifr.ifr_flags & IFF_LOOPBACK)
			continue;

  		if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0)
			continue;

		aiAddresses[iAddresses][6] = 0;
		aiAddresses[iAddresses][7] = 0;
  		bcopy( ifr.ifr_hwaddr.sa_data, aiAddresses[iAddresses++], 6);
	}

	close(s);
}

void GetScreenSize(int& iWidth, int& iHeight)
{
	Display* pdsp = NULL;
	Window wid = 0;
	XWindowAttributes xwAttr;

	pdsp = XOpenDisplay( NULL );
	if ( !pdsp )
		return;

	wid = DefaultRootWindow( pdsp );
	if ( 0 > wid )
		return;
 
	Status ret = XGetWindowAttributes( pdsp, wid, &xwAttr );
	iWidth = xwAttr.width;
	iHeight = xwAttr.height;

	XCloseDisplay( pdsp );
}

size_t GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void SleepMS(size_t iMS)
{
	usleep(iMS);
}

void OpenBrowser(const tstring& sURL)
{
	int iSystem = system((tstring("firefox ") + sURL).c_str());
}

void OpenExplorer(const tstring& sDirectory)
{
	int iSystem = system((tstring("gnome-open ") + sDirectory).c_str());
}

void Alert(const tstring& sMessage)
{
	fputs(sMessage.c_str(), stderr);
}

void CreateMinidump(void* pInfo, tchar* pszDirectory)
{
}

tstring GetClipboard()
{
}

void SetClipboard(const tstring& sBuf)
{
}

tstring GetAppDataDirectory(const tstring& sDirectory, const tstring& sFile)
{
	char* pszVar = getenv("HOME");

	tstring sSuffix;
	sSuffix.append(".").append(sDirectory).append("/").append(sFile);

	tstring sReturn(pszVar);

	mkdir((tstring(sReturn).append("/").append(".").append(sDirectory)).c_str(), 0777);

	sReturn.append("/").append(sSuffix);
	return sReturn;
}

tvector<tstring> ListDirectory(const tstring& sDirectory, bool bDirectories)
{
	tvector<tstring> asResult;

	struct dirent *dp;

	DIR *dir = opendir((sDirectory).c_str());
	while ((dp=readdir(dir)) != NULL)
	{
		if (!bDirectories && (dp->d_type == DT_DIR))
			continue;

		tstring sName = dp->d_name;
		if (sName == ".")
			continue;

		if (sName == "..")
			continue;

		asResult.push_back(sName);
	}
	closedir(dir);

	return asResult;
}

bool IsFile(const tstring& sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISREG(stFileInfo.st_mode))
		return true;
	else
		return false;
}

bool IsDirectory(const tstring& sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISDIR(stFileInfo.st_mode))
		return true;
	else
		return false;
}

void CreateDirectoryNonRecursive(const tstring& sPath)
{
	TUnimplemented();

	mkdir(sPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool CopyFileTo(const tstring& sFrom, const tstring& sTo, bool bOverride)
{
	TUnimplemented();

	int read_fd;
	int write_fd;
	struct stat stat_buf;
	off_t offset = 0;

	read_fd = open(sFrom.c_str(), O_RDONLY);

	if (!read_fd)
		return false;

	fstat(read_fd, &stat_buf);

	write_fd = open(sTo.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
	if (!write_fd)
	{
		close(read_fd);
		return false;
	}

	sendfile(write_fd, read_fd, &offset, stat_buf.st_size);

	close(read_fd);
	close(write_fd);

	return true;
}

tstring FindAbsolutePath(const tstring& sPath)
{
	TUnimplemented();

	char* pszFullPath = realpath(sPath.c_str(), nullptr);
	tstring sFullPath = pszFullPath;
	free(pszFullPath);

	return sFullPath;
}

time_t GetFileModificationTime(const char* pszFile)
{
	TUnimplemented();

	struct stat s;
	if (stat(pszFile, &s) != 0)
		return 0;

	return s.st_mtime;
}

void DebugPrint(const tstring& sText)
{
	puts(sText.c_str());
}

void Exec(const tstring& sLine)
{
	int iSystem = system((tstring("./") + sLine).c_str());
}

// Not worried about supporting these on Linux right now.
int TranslateKeyToQwerty(int iKey)
{
	return iKey;
}

int TranslateKeyFromQwerty(int iKey)
{
	return iKey;
}



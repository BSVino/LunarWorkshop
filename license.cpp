#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <string.h>

#include <strutils.h>
#include <platform.h>
#include <mtrand.h>

bool GenerateKey(eastl::string sProductCode, eastl::string& sKey)
{
	if (!sProductCode.size())
		return false;

	eastl::vector<eastl::string> sTokens;
	strtok(sProductCode, sTokens, "-");

	int iProductCode = atoi(sTokens[0].c_str());

	unsigned char aiHostID[8];
	memset(aiHostID, 0, sizeof(aiHostID));

	char szOctet[3];
	szOctet[2] = '\0';
	const char* pszOctets = sTokens[1].c_str();
	for (size_t i = 0; i < 8; i++)
	{
		szOctet[0] = pszOctets[(i*2)];
		szOctet[1] = pszOctets[(i*2)+1];
		int iID;
		sscanf(szOctet, "%x", &iID);
		aiHostID[i] = (unsigned char)iID;
	}

	unsigned char szTexture[40] =
	{
		0xb3, 0x5c, 0x5a, 0xdd, 0x83, 0xdf, 0xba, 0xd3, 0xf6, 0x99,
		0x86, 0xd9, 0xb7, 0x9d, 0x1e, 0xf1, 0xec, 0x13, 0x76, 0x00,
		0x2b, 0xb8, 0x69, 0x16, 0x5a, 0x51, 0x9c, 0x5d, 0xdc, 0x14,
		0x34, 0x21, 0x20, 0xf0, 0x94, 0x5b, 0x14, 0xfd, 0x53, 0xdd
	};

	size_t i;

	unsigned int iIdSum = 0;

	for (i = 0; i < 8; i++)
		iIdSum += aiHostID[i];

	mtsrand(iProductCode + iIdSum);

	unsigned char szResult[41];

	for (i = 0; i < 40; i++)
	{
		szResult[i] = (szTexture[i] ^ (unsigned char)mtrand())%36;

		if (szResult[i] < 10)
			szResult[i] += '0';
		else
			szResult[i] += 'A' - 10;

		if (szResult[i] == '1')
			szResult[i] = 'R';

		else if (szResult[i] == 'l')
			szResult[i] = 'T';

		else if (szResult[i] == 'I')
			szResult[i] = '7';

		else if (szResult[i] == '0')
			szResult[i] = '4';

		else if (szResult[i] == 'O')
			szResult[i] = 'Z';
	}

	szResult[40] = '\0';

	sKey = eastl::string((char*)szResult);

	return true;
}

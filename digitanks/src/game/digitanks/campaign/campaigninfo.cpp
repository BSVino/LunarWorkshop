#include "campaigninfo.h"

#include <iostream>
#include <fstream>
#include <assert.h>

#include <strutils.h>

#include <datamanager/dataserializer.h>

CCampaignInfo::CCampaignInfo(eastl::string16 sScript)
{
	std::ifstream f(sScript.c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	ReadFromData(pData);

	delete pData;
}

void CCampaignInfo::ReadFromData(CData* pData)
{
	CData* pName = pData->FindChild("Name");
	if (pName)
		m_sName = pName->GetValueString();

	CData* pLevels = pData->FindChild("Levels");

	if (pLevels)
	{
		for (size_t i = 0; i < pLevels->GetNumChildren(); i++)
		{
			CData* pChild = pLevels->FindChild(convertstring<char16_t, char>(sprintf(L"%d", i+1)));

			assert(pChild);
			if (!pChild)
				continue;

			m_asLevels.push_back(pChild->GetValueString());
		}
	}
}

const CCampaignInfo* CCampaignInfo::GetCampaignInfo()
{
	static CCampaignInfo gInfo(L"scripts/campaign.txt");

	return &gInfo;
}
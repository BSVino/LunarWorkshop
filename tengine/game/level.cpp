#include "level.h"

#include <datamanager/data.h>

void CLevel::ReadInfoFromData(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		OnReadInfo(pChildData);
	}
}

void CLevel::OnReadInfo(const CData* pData)
{
	if (pData->GetKey() == "Name")
		m_sName = pData->GetValueTString();
	else if (pData->GetKey() == "GameMode")
		m_sGameMode = pData->GetValueTString();
}

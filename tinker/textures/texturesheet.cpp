#include "texturesheet.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <datamanager/dataserializer.h>
#include <renderer/renderer.h>
#include <textures/texturelibrary.h>

CTextureSheet::CTextureSheet(tstring sFile)
{
	std::basic_ifstream<tchar> f(sFile.c_str());

	CData* pFile = new CData();
	CDataSerializer::Read(f, pFile);

	for (size_t i = 0; i < pFile->GetNumChildren(); i++)
	{
		CData* pChild = pFile->GetChild(i);
		if (pChild->GetKey() == "Texture")
		{
			tstring sTexture = pChild->GetValueString();
			m_hDefaultSheet = CTextureLibrary::AddTexture(sTexture);
		}
		else if (pChild->GetKey() == "Area")
		{
			int x = 0;
			int y = 0;
			int w = 0;
			int h = 0;

			CData* pData = pChild->FindChild("x");
			if (pData)
				x = pData->GetValueInt();

			pData = pChild->FindChild("y");
			if (pData)
				y = pData->GetValueInt();

			pData = pChild->FindChild("w");
			if (pData)
				w = pData->GetValueInt();

			pData = pChild->FindChild("h");
			if (pData)
				h = pData->GetValueInt();

			m_aAreas[pChild->GetValueString()].m_rRect = Rect(x, y, w, h);

			m_aAreas[pChild->GetValueString()].m_hSheet.Reset();
			pData = pChild->FindChild("Texture");
			if (pData)
			{
				tstring sTexture = pData->GetValueString();
				m_aAreas[pChild->GetValueString()].m_hSheet = CTextureLibrary::AddTexture(sTexture);
			}
		}
	}

	delete pFile;
}

CTextureSheet::~CTextureSheet()
{
}

const Rect& CTextureSheet::GetArea(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
	{
		static Rect empty(0,0,0,0);
		return empty;
	}

	return it->second.m_rRect;
}

CTextureHandle CTextureSheet::GetSheet(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return CTextureHandle();

	const CTextureHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
		return m_hDefaultSheet;

	return hSheet;
}

size_t CTextureSheet::GetSheetWidth(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	const CTextureHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
		return m_hDefaultSheet->m_iWidth;

	return it->second.m_hSheet->m_iWidth;
}

size_t CTextureSheet::GetSheetHeight(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	const CTextureHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
		return m_hDefaultSheet->m_iHeight;

	return it->second.m_hSheet->m_iHeight;
}

#ifndef DT_TEXTURE_SHEET_H
#define DT_TEXTURE_SHEET_H

#include <EASTL/map.h>

#include <tstring.h>
#include <geometry.h>

#include "texturehandle.h"

class CTextureArea
{
public:
	Rect					m_rRect;
	CTextureHandle			m_hSheet;
};

class CTextureSheet
{
public:
							CTextureSheet(tstring sFile);
							~CTextureSheet();

public:
	const Rect&				GetArea(const tstring& sArea) const;
	CTextureHandle			GetSheet(const tstring& sArea) const;
	size_t					GetSheetWidth(const tstring& sArea) const;
	size_t					GetSheetHeight(const tstring& sArea) const;

protected:
	eastl::map<tstring, CTextureArea>	m_aAreas;
	CTextureHandle			m_hDefaultSheet;
};

#endif

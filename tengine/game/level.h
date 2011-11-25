#ifndef LW_LEVEL_H
#define LW_LEVEL_H

#include <tstring.h>

class CLevel
{
public:
	virtual					~CLevel() {};

public:
	void					ReadFromData(const class CData* pData);
	virtual void			OnReadData(const class CData* pData);

	const tstring&			GetName() { return m_sName; }
	const tstring&			GetFile() { return m_sFile; }

	void					SetFile(const tstring& sFile) { m_sFile = sFile; }

	const tstring&			GetGameMode() { return m_sGameMode; }

protected:
	tstring					m_sName;
	tstring					m_sFile;

	tstring					m_sGameMode;
};

#endif

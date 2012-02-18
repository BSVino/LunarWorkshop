#pragma once

#include "grotto_character.h"

#include "mirror.h"

class CToken;

class CPlayerCharacter : public CGrottoCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CGrottoCharacter);

public:
	void						Precache();
	void						Spawn();

	bool						ShouldRender() const { return true; };
	void						PreRender(bool bTransparent) const;

	void						PlaceMirror(mirror_t eMirror);

	CMirror*					GetMirror() const;

	void						FindItems();
	void						DropToken();
	void						PickUpToken(CToken* pToken);
	CToken*						GetToken() const;

	virtual void				Reflected(reflection_t eReflectionType);

	void						GoIntoScreen();
	void						GoOutOfScreen();

protected:
	CEntityHandle<CMirror>		m_hMirror;
	CEntityHandle<CToken>		m_hToken;
};

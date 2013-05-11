#ifndef REFLECTION_PLAYER_CHARACTER_H
#define REFLECTION_PLAYER_CHARACTER_H

#include "reflection_character.h"

#include "mirror.h"

class CToken;

class CPlayerCharacter : public CReflectionCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CReflectionCharacter);

public:
	void						Precache();
	void						Spawn();

	void						PlaceMirror(mirror_t eMirror);

	CMirror*					GetMirror() const;

	void						FindItems();
	void						DropToken();
	void						PickUpToken(CToken* pToken);
	CToken*						GetToken() const;

	virtual void				Reflected(reflection_t eReflectionType);

protected:
	CEntityHandle<CMirror>		m_hMirror;
	CEntityHandle<CToken>		m_hToken;
};

#endif

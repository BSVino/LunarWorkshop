#pragma once

#include "grotto_character.h"

#include "mirror.h"

class CToken;
class CCharacterCamera;

class CPlayerCharacter : public CGrottoCharacter
{
	REGISTER_ENTITY_CLASS(CPlayerCharacter, CGrottoCharacter);

public:
	void						Precache();
	void						Spawn();

	void						PlaceMirror(mirror_t eMirror);

	CMirror*					GetMirror() const;

	CBaseEntity*                Use();
	void						DropToken();
	void						PickUpToken(CToken* pToken);
	CToken*						GetToken() const;

	virtual void				Reflected(reflection_t eReflectionType);

	virtual TFloat              CharacterAcceleration() { return 20.0f; }

protected:
	CEntityHandle<CCharacterCamera> m_hCamera;

	CEntityHandle<CMirror>		m_hMirror;
	CEntityHandle<CToken>		m_hToken;
};

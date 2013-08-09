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

	void                        Think();

	void						PlaceMirror(mirror_t eMirror);

	CMirror*					GetMirror() const;

	CBaseEntity*                Use();
	void						DropToken();
	void						PickUpToken(CToken* pToken);
	CToken*						GetToken() const;

	virtual void				Reflected(reflection_t eReflectionType);

	bool                        DraggingMirror() const;
	void                        DragMirror(CMirror* pMirror);
	void                        ReleaseMirror();

	virtual TFloat              CharacterAcceleration() { return 20.0f; }

protected:
	CEntityHandle<CCharacterCamera> m_hCamera;

	CEntityHandle<CMirror>		m_hMirror;
	CEntityHandle<CToken>		m_hToken;

	CEntityHandle<CMirror>      m_hDraggingMirror;
	Vector                      m_vecDraggingPlayerStart;
	Vector                      m_vecDraggingMirrorStart;
};

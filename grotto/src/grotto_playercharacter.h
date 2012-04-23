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
	virtual Matrix4x4			GetRenderTransform() const;

	virtual TFloat				EyeHeight() const;
	virtual TFloat				BaseCharacterSpeed() { return 3.0f; }
	virtual TFloat				JumpStrength() { return 3.0f; }

	void						PlaceMirror(mirror_t eMirror);

	CMirror*					GetMirror() const;

	void						FindItems();
	void						DropToken();
	void						PickUpToken(CToken* pToken);
	CToken*						GetToken() const;

	virtual void				Reflected(reflection_t eReflectionType);

	void						GoIntoScreen();
	void						GoOutOfScreen();
	void						FlipScreen();

protected:
	CEntityHandle<CMirror>		m_hMirror;
	CEntityHandle<CToken>		m_hToken;
};

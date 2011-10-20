#include "tack_character.h"

#include <tinker/application.h>
#include <renderer/renderingcontext.h>

#include "tack_game.h"
#include "tack_renderer.h"

REGISTER_ENTITY(CTackCharacter);

NETVAR_TABLE_BEGIN(CTackCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTackCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTackCharacter);
INPUTS_TABLE_END();

void CTackCharacter::Spawn()
{
	BaseClass::Spawn();

	SetGlobalGravity(Vector(0, -9.8f, 0));
	m_flMaxStepSize = 0.1f;
}

void CTackCharacter::PostRender(bool bTransparent) const
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.Translate(GetGlobalOrigin());
	c.Scale(0.5f, 2, 0.5f);
	c.RenderSphere();
}

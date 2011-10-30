#include "reflection_character.h"

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>

#include "reflection_game.h"
#include "reflection_renderer.h"

REGISTER_ENTITY(CReflectionCharacter);

NETVAR_TABLE_BEGIN(CReflectionCharacter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CReflectionCharacter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CReflectionCharacter);
INPUTS_TABLE_END();

CReflectionCharacter::CReflectionCharacter()
{
}

void CReflectionCharacter::Spawn()
{
	BaseClass::Spawn();

	SetGlobalGravity(Vector(0, -9.8f, 0));
	m_flMaxStepSize = 0.1f;
}

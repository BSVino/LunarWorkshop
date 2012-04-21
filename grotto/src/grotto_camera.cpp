#include "grotto_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <game/entities/game.h>
#include <tinker/cvar.h>

#include "grotto_character.h"
#include "grotto_game.h"
#include "grotto_renderer.h"

REGISTER_ENTITY(CGrottoCamera);

NETVAR_TABLE_BEGIN(CGrottoCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CGrottoCamera);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoCamera);
INPUTS_TABLE_END();

void CGrottoCamera::Spawn()
{
	BaseClass::Spawn();
}

void CGrottoCamera::Think()
{
	if (!m_hCameraTarget)
		m_hCameraTarget = GrottoGame()->GetLocalPlayerCharacter();

	BaseClass::Think();
}

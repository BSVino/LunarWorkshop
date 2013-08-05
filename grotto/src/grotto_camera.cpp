#include "grotto_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/entities/game.h>
#include <tinker/cvar.h>

#include "grotto_character.h"
#include "grotto_game.h"
#include "grotto_renderer.h"

REGISTER_ENTITY(CGrottoCamera);

NETVAR_TABLE_BEGIN(CGrottoCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrottoCamera);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrottoCamera);
INPUTS_TABLE_END();

CGrottoCamera::CGrottoCamera()
{
}

void CGrottoCamera::Think()
{
	BaseClass::Think();
}

float CGrottoCamera::GetCameraFOV()
{
	return 80;
}

const Vector CGrottoCamera::GetUpVector() const
{
	CCharacter* pCharacter = GetCharacter();
	if (!pCharacter)
		return BaseClass::GetUpVector();

	return pCharacter->GetUpVector();
}

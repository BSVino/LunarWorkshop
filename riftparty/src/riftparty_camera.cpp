#include "riftparty_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/entities/game.h>
#include <tinker/cvar.h>

#include "riftparty_character.h"
#include "riftparty_game.h"
#include "riftparty_renderer.h"

REGISTER_ENTITY(CRiftPartyCamera);

NETVAR_TABLE_BEGIN(CRiftPartyCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CRiftPartyCamera);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CRiftPartyCamera);
INPUTS_TABLE_END();

CRiftPartyCamera::CRiftPartyCamera()
{
}

void CRiftPartyCamera::Think()
{
	BaseClass::Think();
}

float CRiftPartyCamera::GetCameraFOV()
{
	return 80;
}

const Vector CRiftPartyCamera::GetUpVector() const
{
	CCharacter* pCharacter = GetCharacter();
	if (!pCharacter)
		return BaseClass::GetUpVector();

	return pCharacter->GetUpVector();
}

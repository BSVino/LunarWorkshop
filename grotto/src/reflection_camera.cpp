#include "reflection_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/game.h>
#include <tinker/cvar.h>

#include "reflection_character.h"
#include "reflection_game.h"
#include "reflection_renderer.h"

CReflectionCamera::CReflectionCamera()
{
}

void CReflectionCamera::Think()
{
	if (ReflectionGame()->GetNumLocalPlayers())
		m_hCharacter = ReflectionGame()->GetLocalPlayerCharacter();
	else
		m_hCharacter = nullptr;

	BaseClass::Think();
}

Vector CReflectionCamera::GetCameraPosition()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraPosition();

	return BaseClass::GetCameraPosition();
}

Vector CReflectionCamera::GetCameraDirection()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraDirection();

	return BaseClass::GetCameraDirection();
}

TVector CReflectionCamera::GetCameraUp()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraUp();

	return BaseClass::GetCameraUp();
}

float CReflectionCamera::GetCameraFOV()
{
	return 80;
}

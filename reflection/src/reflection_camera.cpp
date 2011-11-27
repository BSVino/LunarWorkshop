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

Vector CReflectionCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	if (CVar::GetCVarValue("game_mode") == "menu")
		return BaseClass::GetCameraPosition();

	CReflectionCharacter* pCharacter = ReflectionGame()->GetLocalPlayerCharacter();
	if (!pCharacter)
		return BaseClass::GetCameraPosition();

	return pCharacter->GetGlobalOrigin() + pCharacter->EyeHeight() * pCharacter->GetUpVector();
}

Vector CReflectionCamera::GetCameraTarget()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	if (CVar::GetCVarValue("game_mode") == "menu")
		return BaseClass::GetCameraPosition();

	CReflectionCharacter* pCharacter = ReflectionGame()->GetLocalPlayerCharacter();
	if (!pCharacter)
		return BaseClass::GetCameraPosition();

	return pCharacter->GetGlobalOrigin() + pCharacter->EyeHeight() * pCharacter->GetUpVector() + AngleVector(pCharacter->GetViewAngles());
}

TVector CReflectionCamera::GetCameraUp()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return BaseClass::GetCameraPosition();

	CReflectionCharacter* pCharacter = ReflectionGame()->GetLocalPlayerCharacter();
	if (!pCharacter)
		return BaseClass::GetCameraUp();

	return pCharacter->GetUpVector();
}

float CReflectionCamera::GetCameraFOV()
{
	return 60;
}

#include "grotto_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <game/entities/game.h>
#include <tinker/cvar.h>

#include "grotto_character.h"
#include "grotto_game.h"
#include "grotto_renderer.h"

CGrottoCamera::CGrottoCamera()
{
	m_bThirdPerson = true;

	CVar::SetCVar("cam_third_back", 40.0f);
	CVar::SetCVar("cam_third_right", 0.0f);
}

void CGrottoCamera::Think()
{
	if (GrottoGame()->GetNumLocalPlayers())
		m_hCharacter = GrottoGame()->GetLocalPlayerCharacter();
	else
		m_hCharacter = nullptr;

	BaseClass::Think();
}

Vector CGrottoCamera::GetCameraPosition()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraPosition();

	return BaseClass::GetCameraPosition();
}

Vector CGrottoCamera::GetCameraDirection()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraDirection();

	return BaseClass::GetCameraDirection();
}

TVector CGrottoCamera::GetCameraUp()
{
	if (CVar::GetCVarValue("game_mode") == "menu")
		return CCamera::GetCameraUp();

	return BaseClass::GetCameraUp();
}

float CGrottoCamera::GetCameraFOV()
{
	return 40;
}

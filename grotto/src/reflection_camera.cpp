#include "reflection_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/entities/game.h>
#include <tinker/cvar.h>

#include "reflection_character.h"
#include "reflection_game.h"
#include "reflection_renderer.h"

CReflectionCamera::CReflectionCamera()
{
}

void CReflectionCamera::Think()
{
	BaseClass::Think();
}

float CReflectionCamera::GetCameraFOV()
{
	return 80;
}

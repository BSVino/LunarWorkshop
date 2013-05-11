#include "grotto_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/entities/game.h>
#include <tinker/cvar.h>

#include "grotto_character.h"
#include "grotto_game.h"
#include "grotto_renderer.h"

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

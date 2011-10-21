#include "tack_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/game.h>
#include <tinker/cvar.h>

#include "characters/tack_character.h"
#include "tack_game.h"
#include "tack_renderer.h"

CTackCamera::CTackCamera()
{
}

CVar cam_distance("cam_distance", "12");
CVar cam_pitch("cam_pitch", "45");

Vector CTackCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	CTackCharacter* pCharacter = TackGame()->GetLocalPlayerCharacter();
	if (!pCharacter)
		return Vector(10,0,0);

	return pCharacter->GetGlobalOrigin() + AngleVector(EAngle(cam_pitch.GetFloat(), -180, 0)) * cam_distance.GetFloat();
}

Vector CTackCamera::GetCameraTarget()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraTarget();

	CTackCharacter* pCharacter = TackGame()->GetLocalPlayerCharacter();

	if (!pCharacter)
		return Vector();

	return pCharacter->GetGlobalOrigin();
}

float CTackCamera::GetCameraFOV()
{
	return 60;
}

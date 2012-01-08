#include "chain_camera.h"

#include <matrix.h>

#include <tinker/application.h>
#include <tengine/game/game.h>
#include <tinker/cvar.h>

#include "chain_game.h"
#include "chain_renderer.h"
#include "chain_playercharacter.h"

CChainCamera::CChainCamera()
{
}

Vector CChainCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	CPlayerCharacter* pCharacter = ChainGame()->GetLocalPlayerCharacter();
	if (!pCharacter)
		return Vector(0, 0, 10);

	return Vector(0, 0, 10);
}

Vector CChainCamera::GetCameraDirection()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraDirection();

	return Vector(0, 0, -1);
}

float CChainCamera::GetCameraFOV()
{
	return 45;
}

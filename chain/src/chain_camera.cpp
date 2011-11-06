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
		return Vector(10,0,0);

	return Vector(0, 0, 10);
}

Vector CChainCamera::GetCameraTarget()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraTarget();

	return Vector(0, 0, 0);
}

float CChainCamera::GetCameraFOV()
{
	return 45;
}

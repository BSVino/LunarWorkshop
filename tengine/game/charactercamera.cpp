#include "charactercamera.h"

#include <tinker/cvar.h>
#include <game/entities/character.h>

CCharacterCamera::CCharacterCamera()
{
	m_bThirdPerson = false;
}

TVector CCharacterCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	CCharacter* pCharacter = m_hCharacter;
	if (!pCharacter)
		return BaseClass::GetCameraPosition();

	if (GetThirdPerson())
		return GetThirdPersonCameraPosition();

	return pCharacter->GetGlobalOrigin() + pCharacter->EyeHeight() * pCharacter->GetUpVector();
}

TVector CCharacterCamera::GetCameraDirection()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraDirection();

	CCharacter* pCharacter = m_hCharacter;
	if (!pCharacter)
		return BaseClass::GetCameraDirection();

	if (GetThirdPerson())
		return GetThirdPersonCameraDirection();

	return AngleVector(pCharacter->GetViewAngles());
}

TVector CCharacterCamera::GetCameraUp()
{
	CCharacter* pCharacter = m_hCharacter;
	if (!pCharacter)
		return BaseClass::GetCameraPosition();

	return pCharacter->GetUpVector();
}

CVar cam_third_back("cam_third_back", "1");
CVar cam_third_right("cam_third_right", "0.2");

TVector CCharacterCamera::GetThirdPersonCameraPosition()
{
	CCharacter* pCharacter = m_hCharacter;
	if (!pCharacter)
		return TVector(10, 0, 0);

	TVector vecEyeHeight = pCharacter->GetUpVector() * pCharacter->EyeHeight();

	TMatrix mView = TMatrix(pCharacter->GetThirdPersonCameraAngles(), TVector());
	TVector vecThird = pCharacter->GetGlobalTransform().GetTranslation() + vecEyeHeight;
	vecThird -= mView.GetForwardVector() * cam_third_back.GetFloat();
	vecThird += mView.GetRightVector() * cam_third_right.GetFloat();

	return vecThird;
}

TVector CCharacterCamera::GetThirdPersonCameraDirection()
{
	CCharacter* pCharacter = m_hCharacter;

	if (!pCharacter)
		return TVector();

	return AngleVector(pCharacter->GetThirdPersonCameraAngles());
}

void CCharacterCamera::KeyDown(int c)
{
	BaseClass::KeyDown(c);

	if (c == 'X')
		ToggleThirdPerson();
}

void CCharacterCamera::ToggleThirdPerson()
{
	SetThirdPerson(!GetThirdPerson());
}

#include "manipulator.h"

#include <geometry.h>

#include <tinker/application.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>

void CManipulatorTool::Activate(IManipulatorListener* pListener, const TRS& trs, const tstring& sArguments)
{
	m_bActive = true;
	m_pListener = pListener;
	m_trsTransform = trs;
	m_sListenerArguments = sArguments;
}

void CManipulatorTool::Deactivate()
{
	m_bActive = false;
}

bool CManipulatorTool::MouseInput(int iButton, int iState)
{
	if (!iState)
	{
		if (m_bTransforming)
		{
			m_bTransforming = false;

			TRS trsNewTransform = GetNewTRS();
			if (!(m_iLockedAxis & (1<<0)))
				m_trsTransform.m_vecTranslation.x = trsNewTransform.m_vecTranslation.x;
			if (!(m_iLockedAxis & (1<<1)))
				m_trsTransform.m_vecTranslation.y = trsNewTransform.m_vecTranslation.y;
			if (!(m_iLockedAxis & (1<<2)))
				m_trsTransform.m_vecTranslation.z = trsNewTransform.m_vecTranslation.z;

			m_pListener->ManipulatorUpdated(m_sListenerArguments);

			return true;
		}

		return false;
	}

	int x, y;
	Application()->GetMousePosition(x, y);

	Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	float flClosest = -1;
	m_iLockedAxis = -1;

	if (DistanceToLine(m_trsTransform.m_vecTranslation, vecPosition, vecCamera) < 0.2f)
	{
		m_flOriginalDistance = (m_trsTransform.m_vecTranslation - vecCamera).Length();
		m_iLockedAxis = 0;
	}

	if (DistanceToLine(m_trsTransform.m_vecTranslation + Vector(1, 0, 0), vecPosition, vecCamera) < 0.1f)
	{
		float flDistance = (m_trsTransform.m_vecTranslation + Vector(1, 0, 0) - vecCamera).Length();
		if (m_iLockedAxis < 0 || flDistance < m_flOriginalDistance)
		{
			m_flOriginalDistance = flDistance;
			m_iLockedAxis = (1<<1)|(1<<2);
		}
	}

	if (DistanceToLine(m_trsTransform.m_vecTranslation + Vector(0, 1, 0), vecPosition, vecCamera) < 0.1f)
	{
		float flDistance = (m_trsTransform.m_vecTranslation + Vector(0, 1, 0) - vecCamera).Length();
		if (m_iLockedAxis < 0 || flDistance < m_flOriginalDistance)
		{
			m_flOriginalDistance = flDistance;
			m_iLockedAxis = (1<<0)|(1<<2);
		}
	}

	if (DistanceToLine(m_trsTransform.m_vecTranslation + Vector(0, 0, 1), vecPosition, vecCamera) < 0.1f)
	{
		float flDistance = (m_trsTransform.m_vecTranslation + Vector(0, 0, 1) - vecCamera).Length();
		if (m_iLockedAxis < 0 || flDistance < m_flOriginalDistance)
		{
			m_flOriginalDistance = flDistance;
			m_iLockedAxis = (1<<0)|(1<<1);
		}
	}

	if (m_iLockedAxis >= 0)
	{
		m_flStartX = (float)x;
		m_flStartY = (float)y;
		m_bTransforming = true;
		return true;
	}

	return false;
}

Matrix4x4 CManipulatorTool::GetTransform(bool bScale)
{
	if (m_bTransforming)
		return GetNewTRS().GetMatrix4x4();

	return m_trsTransform.GetMatrix4x4(bScale);
}

TRS CManipulatorTool::GetNewTRS()
{
	int x, y;
	Application()->GetMousePosition(x, y);

	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	Vector vecOldPosition = GameServer()->GetRenderer()->WorldPosition(Vector(m_flStartX, m_flStartY, 1));
	Vector vecNewPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecOldTranslation = vecCamera + (vecOldPosition - vecCamera).Normalized() * m_flOriginalDistance;
	Vector vecNewTranslation = vecCamera + (vecNewPosition - vecCamera).Normalized() * m_flOriginalDistance;

	TRS trs = m_trsTransform;

	Vector vecTranslation = trs.m_vecTranslation + (vecNewTranslation - vecOldTranslation);

	if (!(m_iLockedAxis & (1<<0)))
		trs.m_vecTranslation.x = vecTranslation.x;
	if (!(m_iLockedAxis & (1<<1)))
		trs.m_vecTranslation.y = vecTranslation.y;
	if (!(m_iLockedAxis & (1<<2)))
		trs.m_vecTranslation.z = vecTranslation.z;

	return trs;
}

CManipulatorTool* CManipulatorTool::s_pManipulatorTool = nullptr;

CManipulatorTool* CManipulatorTool::Get()
{
	if (!s_pManipulatorTool)
		s_pManipulatorTool = new CManipulatorTool();

	return s_pManipulatorTool;
}

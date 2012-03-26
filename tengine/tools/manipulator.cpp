#include "manipulator.h"

#include <geometry.h>

#include <tinker/application.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>

void CManipulatorTool::Activate(IManipulatorListener* pListener, const TRS& trs)
{
	m_bActive = true;
	m_pListener = pListener;
	m_trsTransform = trs;
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

			int x, y;
			Application()->GetMousePosition(x, y);

			Vector vecNewPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
			Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

			m_trsTransform.m_vecTranslation = vecCamera + (vecNewPosition - vecCamera).Normalized() * m_flOriginalDistance;

			return true;
		}

		return false;
	}

	int x, y;
	Application()->GetMousePosition(x, y);

	Vector vecPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
	Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

	if (DistanceToLine(m_trsTransform.m_vecTranslation, vecPosition, vecCamera) < 0.2f)
	{
		m_flStartX = (float)x;
		m_flStartY = (float)y;
		m_flOriginalDistance = (m_trsTransform.m_vecTranslation - vecCamera).Length();
		m_bTransforming = true;
		return true;
	}

	return false;
}

Matrix4x4 CManipulatorTool::GetTransform(bool bScale)
{
	if (m_bTransforming)
	{
		int x, y;
		Application()->GetMousePosition(x, y);

		Vector vecNewPosition = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));
		Vector vecCamera = GameServer()->GetRenderer()->GetCameraPosition();

		TRS trs = m_trsTransform;
		trs.m_vecTranslation = vecCamera + (vecNewPosition - vecCamera).Normalized() * m_flOriginalDistance;

		return trs.GetMatrix4x4();
	}

	return m_trsTransform.GetMatrix4x4(bScale);
}

CManipulatorTool* CManipulatorTool::s_pManipulatorTool = nullptr;

CManipulatorTool* CManipulatorTool::Get()
{
	if (!s_pManipulatorTool)
		s_pManipulatorTool = new CManipulatorTool();

	return s_pManipulatorTool;
}

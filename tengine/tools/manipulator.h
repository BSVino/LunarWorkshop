#pragma once

#include "trs.h"

class IManipulatorListener
{
public:
	virtual void		ManipulatorUpdated()=0;
};

class CManipulatorTool
{
public:
	CManipulatorTool()
	{
		m_bActive = false;
		m_pListener = nullptr;
		m_bTransforming = false;
	}

public:
	void				Activate(IManipulatorListener* pListener, const TRS& trs=TRS());
	void				Deactivate();
	bool				IsActive() { return m_bActive; }

	bool				MouseInput(int iButton, int iState);

	Matrix4x4			GetTransform(bool bScale = true);

	TRS					GetNewTRS();

protected:
	bool				m_bActive;
	bool				m_bTransforming;

	char				m_iLockedAxis;
	float				m_flStartX;
	float				m_flStartY;
	float				m_flOriginalDistance;

	TRS					m_trsTransform;

	IManipulatorListener*	m_pListener;

public:
	static CManipulatorTool*	Get();

protected:
	static CManipulatorTool*	s_pManipulatorTool;
};

inline CManipulatorTool* Manipulator()
{
	return CManipulatorTool::Get();
}

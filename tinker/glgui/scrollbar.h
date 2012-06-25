#pragma once

#include "basecontrol.h"

namespace glgui
{
	class CScrollBar : public CBaseControl
	{
		DECLARE_CLASS(CScrollBar, CBaseControl);

	public:
						CScrollBar(bool bHorizontal);	// If it's not horizontal it's vertical

	public:
		virtual void	Layout();
		virtual void	Paint(float x, float y, float w, float h);

		virtual void	SetVisible(bool bVisible);

		virtual void	Think();

		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);
		virtual bool	IsCursorListener() {return true;};

		virtual void	CursorOut();

		virtual void	DoneMovingHandle();

		virtual float	HandleX();
		virtual float	HandleY();

		virtual float	GetHandlePosition() const { return m_flHandlePosition; }

	protected:
		bool			m_bHorizontal;

		float			m_flHandlePosition;
		float			m_flHandlePositionGoal;
		bool			m_bMovingHandle;

		float			m_flHandleSize;
		float			m_flHandleGrabOffset;
	};
}

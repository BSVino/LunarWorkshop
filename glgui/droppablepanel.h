#ifndef TINKER_DROPPABLE_PANEL_H
#define TINKER_DROPPABLE_PANEL_H

#include "panel.h"

namespace glgui
{
	class CDroppablePanel : public CPanel, public IDroppable
	{
	public:
							CDroppablePanel(float x, float y, float w, float h);
		virtual				~CDroppablePanel();

	public:
		// It's already in CBaseControl, but we need this again for IDroppable.
		virtual CBaseControl*	GetParent() { return CPanel::GetParent(); };
		virtual void		SetParent(CBaseControl* pParent) { return CPanel::SetParent(pParent); };
		virtual bool		IsVisible() { return CPanel::IsVisible(); };

		virtual void		Paint(float x, float y, float w, float h);

		virtual void		SetSize(float w, float h);
		virtual void		SetPos(float x, float y);

		virtual bool		MousePressed(int code, int mx, int my);

		virtual void		AddDraggable(IDraggable* pDragged);
		virtual void		SetDraggable(IDraggable* pDragged, bool bDelete = true);
		virtual void		ClearDraggables(bool bDelete = true);
		virtual IDraggable*	GetDraggable(int i);

		virtual void		SetGrabbale(bool bGrabbable) { m_bGrabbable = bGrabbable; };
		virtual bool		IsGrabbale() { return m_bGrabbable; };

	protected:
		bool				m_bGrabbable;

		tvector<IDraggable*>	m_apDraggables;
	};
};

#endif

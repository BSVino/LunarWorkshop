#ifndef TINKER_DROPPABLE_PANEL_H
#define TINKER_DROPPABLE_PANEL_H

#include "panel.h"

namespace glgui
{
	class CDroppablePanel : public CPanel, public IDroppable
	{
	public:
							CDroppablePanel(int x, int y, int w, int h);
		virtual				~CDroppablePanel();

	public:
		// It's already in CBaseControl, but we need this again for IDroppable.
		virtual IControl*	GetParent() { return CPanel::GetParent(); };
		virtual void		SetParent(IControl* pParent) { return CPanel::SetParent(pParent); };
		virtual bool		IsVisible() { return CPanel::IsVisible(); };

		virtual void		Paint(int x, int y, int w, int h);

		virtual void		SetSize(int w, int h);
		virtual void		SetPos(int x, int y);

		virtual bool		MousePressed(int code, int mx, int my);

		virtual void		AddDraggable(IDraggable* pDragged);
		virtual void		SetDraggable(IDraggable* pDragged, bool bDelete = true);
		virtual void		ClearDraggables(bool bDelete = true);
		virtual IDraggable*	GetDraggable(int i);

		virtual void		SetGrabbale(bool bGrabbable) { m_bGrabbable = bGrabbable; };
		virtual bool		IsGrabbale() { return m_bGrabbable; };

	protected:
		bool				m_bGrabbable;

		eastl::vector<IDraggable*>	m_apDraggables;
	};
};

#endif

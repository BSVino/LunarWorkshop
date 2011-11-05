#include "droppablepanel.h"

#include "rootpanel.h"

using namespace glgui;

CDroppablePanel::CDroppablePanel(int x, int y, int w, int h)
	: CPanel(x, y, w, h)
{
	m_bGrabbable = true;

	CRootPanel::Get()->AddDroppable(this);
};

CDroppablePanel::~CDroppablePanel()
{
	if (m_apDraggables.size())
	{
		for (size_t i = 0; i < m_apDraggables.size(); i++)
			delete m_apDraggables[i];
	}

	if (CRootPanel::Get())
		CRootPanel::Get()->RemoveDroppable(this);
}

void CDroppablePanel::Paint(int x, int y, int w, int h)
{
	if (!IsVisible())
		return;

	for (size_t i = 0; i < m_apDraggables.size(); i++)
	{
		// Translate this location to the child's local space.
		int ax, ay;
		FRect c = m_apDraggables[i]->GetHoldingRect();
		GetAbsPos(ax, ay);
		m_apDraggables[i]->Paint((int)c.x+x-ax, (int)c.y+y-ay);
	}

	CPanel::Paint(x, y, w, h);
}

void CDroppablePanel::SetSize(int w, int h)
{
	CPanel::SetSize(w, h);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

void CDroppablePanel::SetPos(int x, int y)
{
	CPanel::SetPos(x, y);
	for (size_t i = 0; i < m_apDraggables.size(); i++)
		m_apDraggables[i]->SetHoldingRect(GetHoldingRect());
}

bool CDroppablePanel::MousePressed(int code, int mx, int my)
{
	if (!IsVisible())
		return false;

	if (m_bGrabbable && m_apDraggables.size() > 0)
	{
		FRect r = GetHoldingRect();
		if (code == 0 &&
			mx >= r.x &&
			my >= r.y &&
			mx < r.x + r.w &&
			my < r.y + r.h)
		{
			CRootPanel::Get()->DragonDrop(this);
			return true;
		}
	}

	return CPanel::MousePressed(code, mx, my);
}

void CDroppablePanel::SetDraggable(IDraggable* pDragged, bool bDelete)
{
	ClearDraggables(bDelete);

	AddDraggable(pDragged);
}

void CDroppablePanel::AddDraggable(IDraggable* pDragged)
{
	if (pDragged)
	{
		m_apDraggables.push_back(pDragged);
		pDragged->SetHoldingRect(GetHoldingRect());
		pDragged->SetDroppable(this);
	}
}

void CDroppablePanel::ClearDraggables(bool bDelete)
{
	if (bDelete)
	{
		for (size_t i = 0; i < m_apDraggables.size(); i++)
			delete m_apDraggables[i];
	}

	m_apDraggables.clear();
}

IDraggable* CDroppablePanel::GetDraggable(int i)
{
	return m_apDraggables[i];
}

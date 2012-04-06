#include "tree.h"

#include <GL3/gl3w.h>

#include <tinker/shell.h>
#include <renderer/renderingcontext.h>

#include "rootpanel.h"
#include "scrollbar.h"

using namespace glgui;

CTree::CTree(const CTextureHandle& hArrowTexture, const CTextureHandle& hEditTexture, const CTextureHandle& hVisibilityTexture)
	: CPanel(0, 0, 10, 10)
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	m_hArrowTexture = hArrowTexture;
	m_hVisibilityTexture = hVisibilityTexture;
	m_hEditTexture = hEditTexture;

	m_pfnSelectedCallback = NULL;
	m_pSelectedListener = NULL;

	m_pfnDroppedCallback = NULL;
	m_pDroppedListener = NULL;

	m_bMouseDown = false;

	m_pDragging = NULL;

	CRootPanel::Get()->AddDroppable(this);

	SetVerticalScrollBarEnabled(true);
	SetScissoring(true);
}

CTree::~CTree()
{
	CRootPanel::Get()->RemoveDroppable(this);

	// CPanel destructor does this since they are controls.
//	for (size_t i = 0; i < m_apNodes.size(); i++)
//		delete m_apNodes[i];
}

void CTree::Layout()
{
	m_flCurrentHeight = 0;
	m_flCurrentDepth = 0;

	for (size_t i = 0; i < m_apNodes.size(); i++)
		m_apNodes[i]->LayoutNode();

	CPanel::Layout();
}

void CTree::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	m_iHilighted = ~0;
	for (size_t i = 0; i < m_apAllNodes.size(); i++)
	{
		IControl* pNode = m_apAllNodes[i];

		if (!pNode->IsVisible())
			continue;

		float x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iHilighted = i;
			break;
		}
	}

	if (m_bMouseDown && abs(mx - m_iMouseDownX) > 10 && abs(my - m_iMouseDownY) && GetSelectedNode() && !CRootPanel::Get()->GetCurrentDraggable())
	{
		m_pDragging = GetSelectedNode();
		CRootPanel::Get()->DragonDrop(this);
		m_bMouseDown = false;
	}

	CPanel::Think();
}

void CTree::Paint()
{
	float x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CTree::Paint(float x, float y)
{
	Paint(x, y, m_flW, m_flH);
}

void CTree::Paint(float x, float y, float w, float h)
{
	Color clrHilight = g_clrBoxHi;
	clrHilight.SetAlpha(100);
	Color clrSelected = g_clrBoxHi;

	bool bScissor = m_bScissoring;
	float sx, sy;
	if (bScissor)
	{
		GetAbsPos(sx, sy);

		CRootPanel::GetContext()->SetUniform("bScissor", true);
		CRootPanel::GetContext()->SetUniform("vecScissor", Vector4D(sx, sy, GetWidth(), GetHeight()));
	}

	if (m_iHilighted != ~0)
	{
		IControl* pNode = m_apAllNodes[m_iHilighted];
		float cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrHilight, 2);
	}

	if (m_iSelected != ~0 && m_apAllNodes[m_iSelected]->IsVisible())
	{
		IControl* pNode = m_apAllNodes[m_iSelected];
		float cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrSelected, 2);
	}

	CPanel::Paint(x, y, w, h);
}

bool CTree::MousePressed(int code, int mx, int my)
{
	m_bMouseDown = true;
	m_iMouseDownX = mx;
	m_iMouseDownY = my;

	if (CPanel::MousePressed(code, mx, my))
		return true;

	m_iSelected = ~0;
	for (size_t i = 0; i < m_apAllNodes.size(); i++)
	{
		IControl* pNode = m_apAllNodes[i];

		if (!pNode->IsVisible())
			continue;

		float x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iSelected = i;
			CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pNode);
			pTreeNode->Selected();
			return true;
		}
	}

	if (m_pSelectedListener)
		m_pfnSelectedCallback(m_pSelectedListener, "-1");

	return false;
}

bool CTree::MouseReleased(int code, int mx, int my)
{
	m_bMouseDown = false;

	if (CPanel::MouseReleased(code, mx, my))
		return true;

	return false;
}

size_t CTree::AddControl(IControl* pControl, bool bToTail)
{
	size_t iControl = BaseClass::AddControl(pControl, bToTail);

	if (pControl != m_pVerticalScrollBar && pControl != m_pHorizontalScrollBar)
	{
		CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pControl);
		if (pTreeNode)
			m_apAllNodes.push_back(pTreeNode);
	}

	return iControl;
}

void CTree::RemoveControl(IControl* pControl)
{
	BaseClass::RemoveControl(pControl);

	if (pControl != m_pVerticalScrollBar && pControl != m_pHorizontalScrollBar)
	{
		for (size_t i = m_apAllNodes.size()-1; i < m_apAllNodes.size(); i--)
		{
			IControl* pNode = m_apAllNodes[i];

			if (pControl == pNode)
			{
				m_apAllNodes.erase(m_apAllNodes.begin()+i);
				break;
			}
		}
	}
}

void CTree::ClearTree()
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	for (size_t i = m_apAllNodes.size()-1; i < m_apAllNodes.size(); i--)
	{
		IControl* pNode = m_apAllNodes[i];

		RemoveControl(pNode);
		delete pNode;
	}

	m_apNodes.clear();
	m_apAllNodes.clear();
}

size_t CTree::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(NULL, this, sName, "sans-serif"));
}

size_t CTree::AddNode(CTreeNode* pNode, size_t iPosition)
{
	if (iPosition == ~0)
		m_apNodes.push_back(pNode);
	else
		m_apNodes.insert(m_apNodes.begin()+iPosition, pNode);

	AddControl(pNode, true);
	return m_apNodes.size()-1;
}

void CTree::RemoveNode(CTreeNode* pNode)
{
	IControl* pHilighted = NULL;
	IControl* pSelected = NULL;

	// Tuck these away so we can find them again after the controls list has changed.
	if (m_iHilighted != ~0)
		pHilighted = m_apAllNodes[m_iHilighted];
	if (m_iSelected != ~0)
		pSelected = m_apAllNodes[m_iSelected];

	m_iHilighted = ~0;
	m_iSelected = ~0;

	for (size_t i = 0; i < m_apNodes.size(); i++)
	{
		if (m_apNodes[i] == pNode)
		{
			m_apNodes.erase(m_apNodes.begin()+i);
			break;
		}
		else
			m_apNodes[i]->RemoveNode(pNode);
	}

	RemoveControl(pNode);

	// Figure out if our hilighted or selected controls were deleted.
	for (size_t c = 0; c < m_apAllNodes.size(); c++)
	{
		if (m_apAllNodes[c] == pHilighted)
			m_iHilighted = c;
		if (m_apAllNodes[c] == pSelected)
			m_iSelected = c;
	}
}

CTreeNode* CTree::GetNode(size_t i)
{
	return m_apNodes[i];
}

void CTree::SetSelectedNode(size_t iNode)
{
	TAssert(iNode < m_apControls.size() || iNode == ~0);

	if (iNode >= m_apControls.size())
		return;

	m_iSelected = iNode;
}

void CTree::SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pSelectedListener = pListener;
	m_pfnSelectedCallback = pfnCallback;
}

void CTree::SetDroppedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	TAssert(pListener && pfnCallback || !pListener && !pfnCallback);
	m_pDroppedListener = pListener;
	m_pfnDroppedCallback = pfnCallback;
}

void CTree::SetDraggable(IDraggable* pDraggable, bool bDelete)
{
	if (m_pDroppedListener)
		m_pfnDroppedCallback(m_pDroppedListener, "");

	AddNode(dynamic_cast<CTreeNode*>(pDraggable->MakeCopy()));
}

CTreeNode::CTreeNode(CTreeNode* pParent, CTree* pTree, const tstring& sText, const tstring& sFont)
	: CPanel(0, 0, 10, 10)
{
	m_pParent = pParent;
	m_pTree = pTree;

	m_pVisibilityButton = NULL;
	m_pEditButton = NULL;

	m_pLabel = new CLabel(0, 0, GetWidth(), GetHeight(), "");
	m_pLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pLabel->SetText(sText.c_str());
	m_pLabel->SetFont(sFont, 11);
	AddControl(m_pLabel);

	m_pExpandButton = new CExpandButton(m_pTree->m_hArrowTexture);
	m_pExpandButton->SetExpanded(false);
	m_pExpandButton->SetClickedListener(this, Expand);
	AddControl(m_pExpandButton);

	m_bDraggable = false;
}

CTreeNode::CTreeNode(const CTreeNode& c)
	: CPanel(GetLeft(), GetTop(), GetWidth(), GetHeight())
{
	m_pParent = c.m_pParent;
	m_pTree = c.m_pTree;
	m_pVisibilityButton = NULL;
	m_pEditButton = NULL;

	m_pLabel = new CLabel(c.m_pLabel->GetLeft(), c.m_pLabel->GetTop(), c.m_pLabel->GetWidth(), c.m_pLabel->GetHeight(), c.m_pLabel->GetText());
	m_pLabel->SetAlign(c.m_pLabel->GetAlign());
	m_pLabel->SetFont("sans-serif", c.m_pLabel->GetFontFaceSize());
	AddControl(m_pLabel);

	m_pExpandButton = new CExpandButton(m_pTree->m_hArrowTexture);
	m_pExpandButton->SetExpanded(false);
	m_pExpandButton->SetClickedListener(this, Expand);
	AddControl(m_pExpandButton);

	m_hIconTexture = c.m_hIconTexture;
	m_bDraggable = false;
}

CTreeNode::~CTreeNode()
{
	// They are controls of CTree so it will deallocate them.
//	for (size_t i = 0; i < m_apNodes.size(); i++)
//		delete m_apNodes[i];
}

float CTreeNode::GetNodeHeight()
{
	return m_pLabel->GetTextHeight();
}

void CTreeNode::LayoutNode()
{
	float& flCurrentDepth = m_pTree->m_flCurrentDepth;
	float& flCurrentHeight = m_pTree->m_flCurrentHeight;

	float flHeight = GetNodeHeight();

	float x = flCurrentDepth*flHeight;
	float y = flCurrentHeight;
	float w = m_pTree->GetWidth() - flCurrentDepth*flHeight;
	float h = flHeight;

	SetPos(x, y);
	SetSize(w, h);

	m_pLabel->SetHeight(h);
	m_pLabel->SetWidth(w);
	if (m_hIconTexture.IsValid())
		m_pLabel->SetPos(h+12, 0);
	else
		m_pLabel->SetPos(h, 0);

	m_pExpandButton->SetPos(0, 0);
	m_pExpandButton->SetSize(flHeight, flHeight);

	flCurrentHeight += flHeight;
	flCurrentHeight += GetNodeSpacing();

	if (IsExpanded())
	{
		flCurrentDepth += 1;
		for (size_t i = 0; i < m_apNodes.size(); i++)
			m_apNodes[i]->LayoutNode();
		flCurrentDepth -= 1;
	}
}

void CTreeNode::Paint(float x, float y, float w, float h)
{
	Paint(x, y, w, h, false);
}

void CTreeNode::Paint(float x, float y, float w, float h, bool bFloating)
{
	if (!IsVisible())
		return;

	if (m_pTree->m_hArrowTexture.IsValid() && m_apNodes.size())
		m_pExpandButton->Paint();

//	CBaseControl::PaintRect(x+15, y, w-25, h);

	float flIconSize = 0;
	if (m_hIconTexture.IsValid())
	{
		flIconSize = 12;

		PaintTexture(m_hIconTexture, x+12, y, flIconSize, flIconSize);
	}

	m_pLabel->Paint();

	if (m_pVisibilityButton)
		m_pVisibilityButton->Paint();

	if (m_pEditButton)
		m_pEditButton->Paint();

	// Skip CPanel, controls are painted in other ways.
	CBaseControl::Paint(x, y, w, h);
}

size_t CTreeNode::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(this, m_pTree, sName, "sans-serif"));
}

size_t CTreeNode::AddNode(CTreeNode* pNode)
{
	if (!m_apNodes.size())
		SetExpanded(true);
	m_apNodes.push_back(pNode);
	m_pTree->AddControl(pNode);
	return m_apNodes.size()-1;
}

void CTreeNode::RemoveNode(CTreeNode* pNode)
{
	for (size_t i = 0; i < m_apNodes.size(); i++)
	{
		if (m_apNodes[i] == pNode)
		{
			m_apNodes.erase(m_apNodes.begin()+i);
			return;
		}
		m_apNodes[i]->RemoveNode(pNode);
	}
}

CTreeNode* CTreeNode::GetNode(size_t i)
{
	return m_apNodes[i];
}

void CTreeNode::Selected()
{
	if (m_pTree->m_pSelectedListener)
		m_pTree->m_pfnSelectedCallback(m_pTree->m_pSelectedListener, sprintf("%d", m_pTree->GetSelectedNodeId()));
}

bool CTreeNode::IsVisible()
{
	if (!CPanel::IsVisible())
		return false;

	if (!m_pParent)
		return true;

	CTreeNode* pNode = m_pParent;
	do
	{
		if (!pNode->IsExpanded())
			return false;
	}
	while (pNode = pNode->m_pParent);

	return true;
}

void CTreeNode::SetHoldingRect(const FRect&)
{
}

FRect CTreeNode::GetHoldingRect()
{
	return FRect(0, 0, 0, 0);
}

IDroppable* CTreeNode::GetDroppable()
{
	return NULL;
}

void CTreeNode::SetDroppable(IDroppable* pDroppable)
{
}

void CTreeNode::ExpandCallback(const tstring& sArgs)
{
	SetExpanded(!IsExpanded());
	m_pTree->Layout();
}

CTreeNode::CExpandButton::CExpandButton(const CTextureHandle& hTexture)
	: CPictureButton("*", hTexture, false)
{
	m_bExpanded = false;
	m_flExpandedGoal = m_flExpandedCurrent = 0;
}

void CTreeNode::CExpandButton::Think()
{
	m_flExpandedCurrent = Approach(m_flExpandedGoal, m_flExpandedCurrent, (float)CRootPanel::Get()->GetFrameTime()*10);
}

void CTreeNode::CExpandButton::Paint(float x, float y, float w, float h)
{
	TAssert(false);

/*	glPushAttrib(GL_ENABLE_BIT);
	glEnablei(GL_BLEND, 0);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();

	glTranslatef((float)x+w/2, (float)y+h/2, 0);
	glRotatef((m_flExpandedCurrent)*90, 0, 0, 1);

	// Hehe.
	// glRotatef((float)(glutGet(GLUT_ELAPSED_TIME)%3600)/5, 0, 0, 1);

	PaintTexture(m_iTexture, -w/2, -h/2, w, h);

	glPopMatrix();

	glPopAttrib();*/
}

void CTreeNode::CExpandButton::SetExpanded(bool bExpanded)
{
	m_bExpanded = bExpanded;
	m_flExpandedGoal = m_bExpanded?1.0f:0.0f;
}

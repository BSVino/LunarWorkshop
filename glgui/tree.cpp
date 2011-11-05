#include "tree.h"

#include <GL/glew.h>

#include "rootpanel.h"

using namespace glgui;

CTree::CTree(size_t iArrowTexture, size_t iEditTexture, size_t iVisibilityTexture)
	: CPanel(0, 0, 10, 10)
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	m_iArrowTexture = iArrowTexture;
	m_iVisibilityTexture = iVisibilityTexture;
	m_iEditTexture = iEditTexture;

	m_pfnSelectedCallback = NULL;
	m_pSelectedListener = NULL;

	m_pfnDroppedCallback = NULL;
	m_pDroppedListener = NULL;

	m_clrBackground = Color(0, 0, 0);
	m_clrBackground.SetAlpha(0);

	m_bMouseDown = false;

	m_pDragging = NULL;

	CRootPanel::Get()->AddDroppable(this);
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
	m_iCurrentHeight = 0;
	m_iCurrentDepth = 0;

	for (size_t i = 0; i < m_apNodes.size(); i++)
		m_apNodes[i]->LayoutNode();

	CPanel::Layout();
}

void CTree::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	m_iHilighted = ~0;
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		IControl* pNode = m_apControls[i];

		if (!pNode->IsVisible())
			continue;

		int x, y, w, h;
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
	int x = 0, y = 0;
	GetAbsPos(x, y);
	Paint(x, y);
}

void CTree::Paint(int x, int y)
{
	Paint(x, y, m_iW, m_iH);
}

void CTree::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, m_clrBackground);

	Color clrHilight = g_clrBoxHi;
	clrHilight.SetAlpha(100);
	Color clrSelected = g_clrBoxHi;

	if (m_iHilighted != ~0)
	{
		IControl* pNode = m_apControls[m_iHilighted];
		int cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrHilight);
	}

	if (m_iSelected != ~0 && m_apControls[m_iSelected]->IsVisible())
	{
		IControl* pNode = m_apControls[m_iSelected];
		int cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::PaintRect(cx, cy, cw, ch, clrSelected);
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
	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		IControl* pNode = m_apControls[i];

		if (!pNode->IsVisible())
			continue;

		int x, y, w, h;
		pNode->GetAbsDimensions(x, y, w, h);

		if (mx >= x && my >= y && mx < x+w && my < y+h)
		{
			m_iSelected = i;
			CTreeNode* pTreeNode = dynamic_cast<CTreeNode*>(pNode);
			pTreeNode->Selected();
			return true;
		}
	}

	return false;
}

bool CTree::MouseReleased(int code, int mx, int my)
{
	m_bMouseDown = false;

	if (CPanel::MouseReleased(code, mx, my))
		return true;

	return false;
}

void CTree::ClearTree()
{
	m_iHilighted = ~0;
	m_iSelected = ~0;

	while (m_apControls.size())
	{
		IControl* pNode = m_apControls[0];
		RemoveControl(pNode);
		delete pNode;
	}

	m_apNodes.clear();
}

size_t CTree::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(NULL, this, sName, _T("sans-serif")));
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
		pHilighted = m_apControls[m_iHilighted];
	if (m_iSelected != ~0)
		pSelected = m_apControls[m_iSelected];

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
	for (size_t c = 0; c < m_apControls.size(); c++)
	{
		if (m_apControls[c] == pHilighted)
			m_iHilighted = c;
		if (m_apControls[c] == pSelected)
			m_iSelected = c;
	}
}

CTreeNode* CTree::GetNode(size_t i)
{
	return m_apNodes[i];
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
		m_pfnDroppedCallback(m_pDroppedListener);

	AddNode(dynamic_cast<CTreeNode*>(pDraggable->MakeCopy()));
}

CTreeNode::CTreeNode(CTreeNode* pParent, CTree* pTree, const tstring& sText, const tstring& sFont)
	: CPanel(0, 0, 10, 10)
{
	m_pParent = pParent;
	m_pTree = pTree;

	m_pVisibilityButton = NULL;
	m_pEditButton = NULL;

	m_pLabel = new CLabel(0, 0, GetWidth(), GetHeight(), _T(""));
	m_pLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pLabel->SetText(sText.c_str());
	m_pLabel->SetFont(sFont, 11);
	AddControl(m_pLabel);

	m_pExpandButton = new CExpandButton(m_pTree->m_iArrowTexture);
	m_pExpandButton->SetExpanded(false);
	m_pExpandButton->SetClickedListener(this, Expand);
	AddControl(m_pExpandButton);

	m_iIconTexture = 0;

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
	m_pLabel->SetFont(_T("sans-serif"), c.m_pLabel->GetFontFaceSize());
	AddControl(m_pLabel);

	m_pExpandButton = new CExpandButton(m_pTree->m_iArrowTexture);
	m_pExpandButton->SetExpanded(false);
	m_pExpandButton->SetClickedListener(this, Expand);
	AddControl(m_pExpandButton);

	m_iIconTexture = c.m_iIconTexture;
	m_bDraggable = false;
}

CTreeNode::~CTreeNode()
{
	// They are controls of CTree so it will deallocate them.
//	for (size_t i = 0; i < m_apNodes.size(); i++)
//		delete m_apNodes[i];
}

int CTreeNode::GetNodeHeight()
{
	return (int)m_pLabel->GetTextHeight();
}

void CTreeNode::LayoutNode()
{
	int& iCurrentDepth = m_pTree->m_iCurrentDepth;
	int& iCurrentHeight = m_pTree->m_iCurrentHeight;

	int iHeight = GetNodeHeight();

	int iX = iCurrentDepth*iHeight;
	int iY = iCurrentHeight;
	int iW = m_pTree->GetWidth() - iCurrentDepth*iHeight;
	int iH = iHeight;

	SetPos(iX, iY);
	SetSize(iW, iH);

	m_pExpandButton->SetPos(0, 0);
	m_pExpandButton->SetSize(iHeight, iHeight);

	iCurrentHeight += iHeight;
	iCurrentHeight += GetNodeSpacing();

	if (IsExpanded())
	{
		iCurrentDepth++;
		for (size_t i = 0; i < m_apNodes.size(); i++)
			m_apNodes[i]->LayoutNode();
		iCurrentDepth--;
	}
}

void CTreeNode::Paint(int x, int y, int w, int h)
{
	Paint(x, y, w, h, false);
}

void CTreeNode::Paint(int x, int y, int w, int h, bool bFloating)
{
	if (!IsVisible())
		return;

	if (m_pTree->m_iArrowTexture && m_apNodes.size())
		m_pExpandButton->Paint();

//	CBaseControl::PaintRect(x+15, y, w-25, h);

	int iIconSize = 0;
	if (m_iIconTexture)
	{
		iIconSize = 12;

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iIconTexture);
		glColor4f(1,1,1,1);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 1);
			glVertex2d(x+12, y);
			glTexCoord2f(0, 0);
			glVertex2d(x+12, y+iIconSize);
			glTexCoord2f(1, 0);
			glVertex2d(x+12+iIconSize, y+iIconSize);
			glTexCoord2f(1, 1);
			glVertex2d(x+12+iIconSize, y);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		glPopAttrib();
	}

	m_pLabel->Paint(x+h+iIconSize, y, w-h-iIconSize, h);

	if (m_pVisibilityButton)
		m_pVisibilityButton->Paint();

	if (m_pEditButton)
		m_pEditButton->Paint();

	// Skip CPanel, controls are painted in other ways.
	CBaseControl::Paint(x, y, w, h);
}

size_t CTreeNode::AddNode(const tstring& sName)
{
	return AddNode(new CTreeNode(this, m_pTree, sName, _T("sans-serif")));
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
		m_pTree->m_pfnSelectedCallback(m_pTree->m_pSelectedListener);
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

void CTreeNode::ExpandCallback()
{
	SetExpanded(!IsExpanded());
	m_pTree->Layout();
}

CTreeNode::CExpandButton::CExpandButton(size_t iTexture)
	: CPictureButton(_T("*"), iTexture, false)
{
	m_bExpanded = false;
	m_flExpandedGoal = m_flExpandedCurrent = 0;
}

void CTreeNode::CExpandButton::Think()
{
	m_flExpandedCurrent = Approach(m_flExpandedGoal, m_flExpandedCurrent, CRootPanel::Get()->GetFrameTime()*10);
}

void CTreeNode::CExpandButton::Paint(int x, int y, int w, int h)
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();

	glTranslatef((float)x+w/2, (float)y+h/2, 0);
	glRotatef((m_flExpandedCurrent)*90, 0, 0, 1);

	// Hehe.
	// glRotatef((float)(glutGet(GLUT_ELAPSED_TIME)%3600)/5, 0, 0, 1);

	glBindTexture(GL_TEXTURE_2D, (GLuint)m_iTexture);
	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2d(-w/2, -h/2);
		glTexCoord2f(1, 1);
		glVertex2d(-w/2, h/2);
		glTexCoord2f(1, 0);
		glVertex2d(w/2, h/2);
		glTexCoord2f(0, 0);
		glVertex2d(w/2, -h/2);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();

	glPopAttrib();
}

void CTreeNode::CExpandButton::SetExpanded(bool bExpanded)
{
	m_bExpanded = bExpanded;
	m_flExpandedGoal = m_bExpanded?1.0f:0.0f;
}

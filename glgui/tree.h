#ifndef TINKER_TREE_H
#define TINKER_TREE_H

#include "panel.h"
#include "picturebutton.h"

namespace glgui
{
	class CTreeNode : public CPanel, public IDraggable, public glgui::IEventListener
	{
		DECLARE_CLASS(CTreeNode, CPanel);

	public:
											CTreeNode(CTreeNode* pParent, class CTree* pTree, const tstring& sText, const tstring& sFont);
											CTreeNode(const CTreeNode& c);
		virtual								~CTreeNode();

	public:
		virtual float						GetNodeHeight();
		virtual float						GetNodeSpacing() { return 0; };
		virtual void						LayoutNode();
		virtual void						Paint() { CPanel::Paint(); };
		virtual void						Paint(float x, float y) { CPanel::Paint(x, y); };
		virtual void						Paint(float x, float y, float w, float h);
		virtual void						Paint(float x, float y, float w, float h, bool bFloating);

		size_t								AddNode(const tstring& sName);
		template <typename T>
		size_t								AddNode(const tstring& sName, T* pObject);
		size_t								AddNode(CTreeNode* pNode);
		void								RemoveNode(CTreeNode* pNode);
		CTreeNode*							GetNode(size_t i);
		size_t								GetNumNodes() { return m_apNodes.size(); };

		virtual void						AddVisibilityButton() {};

		virtual void						Selected();

		bool								IsExpanded() { return m_pExpandButton->IsExpanded(); };
		void								SetExpanded(bool bExpanded) { m_pExpandButton->SetExpanded(bExpanded); };

		void								SetIcon(size_t iTexture) { m_iIconTexture = iTexture; };
		virtual void						SetDraggable(bool bDraggable) { m_bDraggable = true; };

		virtual bool						IsVisible();

		// IDraggable
		virtual void						SetHoldingRect(const FRect&);
		virtual FRect						GetHoldingRect();

		virtual IDroppable*					GetDroppable();
		virtual void						SetDroppable(IDroppable* pDroppable);

		virtual float						GetWidth() { return BaseClass::GetWidth(); };
		virtual float						GetHeight() { return BaseClass::GetHeight(); };

		virtual DragClass_t					GetClass() { return DC_UNSPECIFIED; };
		virtual IDraggable*					MakeCopy() { return new CTreeNode(*this); };
		virtual bool						IsDraggable() { return m_bDraggable; };

		EVENT_CALLBACK(CTreeNode, Expand);

	public:
		eastl::vector<CTreeNode*>			m_apNodes;
		CTreeNode*							m_pParent;
		class CTree*						m_pTree;
		class CLabel*						m_pLabel;

		size_t								m_iIconTexture;

		CPictureButton*						m_pVisibilityButton;
		CPictureButton*						m_pEditButton;

		bool								m_bDraggable;

		class CExpandButton : public CPictureButton
		{
		public:
											CExpandButton(size_t iTexture);

		public:
			void							Think();
			void							Paint() { CButton::Paint(); };
			void							Paint(float x, float y, float w, float h);

			bool							IsExpanded() { return m_bExpanded; };
			void							SetExpanded(bool bExpanded);

		public:
			bool							m_bExpanded;
			float							m_flExpandedCurrent;
			float							m_flExpandedGoal;
		};

		CExpandButton*						m_pExpandButton;
	};

	class CTree : public CPanel, public IDroppable
	{
		DECLARE_CLASS(CTree, CPanel);

		friend class CTreeNode;

	public:
											CTree(size_t iArrowTexture, size_t iEditTexture, size_t iVisibilityTexture);
		virtual								~CTree();

	public:
		virtual void						Layout();
		virtual void						Think();
		virtual void						Paint();
		virtual void						Paint(float x, float y);
		virtual void						Paint(float x, float y, float w, float h);

		virtual bool						MousePressed(int code, int mx, int my);
		virtual bool						MouseReleased(int iButton, int mx, int my);

		virtual size_t						AddControl(IControl* pControl, bool bToTail = false);
		virtual void						RemoveControl(IControl* pControl);

		void								ClearTree();

		size_t								AddNode(const tstring& sName);
		template <typename T>
		size_t								AddNode(const tstring& sName, T* pObject);
		size_t								AddNode(CTreeNode* pNode, size_t iPosition = ~0);
		void								RemoveNode(CTreeNode* pNode);
		CTreeNode*							GetNode(size_t i);

		virtual CTreeNode*					GetSelectedNode() { if (m_iSelected == ~0) return NULL; return dynamic_cast<CTreeNode*>(m_apControls[m_iSelected]); };
		virtual size_t						GetSelectedNodeId() { return m_iSelected; };
		virtual void						Unselect() { m_iSelected = ~0; }
		virtual void						SetSelectedNode(size_t iNode);
		virtual void						SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual void						SetDroppedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		// IDroppable
		virtual const FRect					GetHoldingRect() { return GetAbsDimensions(); };

		virtual void						AddDraggable(IDraggable*) {};
		virtual void						SetDraggable(IDraggable*, bool bDelete = true);
		virtual IDraggable*					GetDraggable(int i) { return m_pDragging; };
		virtual IDraggable*					GetCurrentDraggable() { return m_pDragging; };

		// I already know.
		virtual void						SetGrabbale(bool bGrabbable) {};
		virtual bool						IsGrabbale() { return true; };

		virtual bool						CanDropHere(IDraggable*) { return true; };

		virtual bool						IsInfinite() { return true; };
		virtual bool						IsVisible() { return BaseClass::IsVisible(); };

	public:
		eastl::vector<CTreeNode*>			m_apNodes;
		eastl::vector<CTreeNode*>			m_apAllNodes;

		float								m_flCurrentHeight;
		float								m_flCurrentDepth;

		size_t								m_iHilighted;
		size_t								m_iSelected;

		size_t								m_iArrowTexture;
		size_t								m_iVisibilityTexture;
		size_t								m_iEditTexture;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;

		IEventListener::Callback			m_pfnDroppedCallback;
		IEventListener*						m_pDroppedListener;

		bool								m_bMouseDown;
		int									m_iMouseDownX;
		int									m_iMouseDownY;

		CTreeNode*							m_pDragging;
		int									m_iAcceptsDragType;
	};

	template <typename T>
	class CTreeNodeObject : public CTreeNode
	{
	public:
		CTreeNodeObject(T* pObject, CTreeNode* pParent, class CTree* pTree, const tstring& sName)
			: CTreeNode(pParent, pTree, sName, "sans-serif")
		{
			m_pObject = pObject;
		}

	public:
		typedef void (*EditFnCallback)(T*, const tstring& sArgs);

		virtual void LayoutNode()
		{
			CTreeNode::LayoutNode();

			float iHeight = m_pLabel->GetTextHeight();

			if (m_pVisibilityButton)
			{
				m_pVisibilityButton->SetPos(GetWidth()-iHeight-14, 0);
				m_pVisibilityButton->SetSize(iHeight, iHeight);
			}

			if (m_pEditButton)
			{
				m_pEditButton->SetPos(GetWidth()-iHeight*2-16, 0);
				m_pEditButton->SetSize(iHeight, iHeight);
			}

			m_pLabel->SetAlpha(m_pObject->IsVisible()?255:100);
		}

		virtual void AddVisibilityButton()
		{
			m_pVisibilityButton = new CPictureButton("@", m_pTree->m_iVisibilityTexture);
			m_pVisibilityButton->SetClickedListener(this, Visibility);
			AddControl(m_pVisibilityButton);
		}

		virtual void AddEditButton(EditFnCallback pfnCallback)
		{
			m_pEditButton = new CPictureButton("*", m_pTree->m_iEditTexture);
			m_pEditButton->SetClickedListener(this, Edit);
			AddControl(m_pEditButton);
			m_pfnCallback = pfnCallback;
		}

		virtual T* GetObject() { return m_pObject; }

		virtual IDraggable*				MakeCopy() { return new CTreeNodeObject<T>(*this); };

		EVENT_CALLBACK(CTreeNodeObject, Visibility);
		EVENT_CALLBACK(CTreeNodeObject, Edit);

	protected:
		T*									m_pObject;

		EditFnCallback						m_pfnCallback;
	};

	template <typename T>
	inline void CTreeNodeObject<T>::VisibilityCallback(const tstring& sArgs)
	{
		m_pObject->SetVisible(!m_pObject->IsVisible());

		m_pLabel->SetAlpha(m_pObject->IsVisible()?255:100);
	}

	template <typename T>
	inline void CTreeNodeObject<T>::EditCallback(const tstring& sArgs)
	{
		m_pfnCallback(m_pObject, sArgs);
	}

	template <typename T>
	inline size_t CTreeNode::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(new CTreeNodeObject<T>(pObject, this, m_pTree, sName));
	}

	template <typename T>
	inline size_t CTree::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(new CTreeNodeObject<T>(pObject, NULL, this, sName));
	}
};

#endif

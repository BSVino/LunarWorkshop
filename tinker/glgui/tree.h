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
											CTreeNode(CControl<CTreeNode> hParent, CControl<CTree> hTree, const tstring& sText, const tstring& sFont);
											CTreeNode(const CTreeNode& c);
		virtual								~CTreeNode();

	public:
		virtual void						CreateControls(CResource<CBaseControl> pThis);

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
		size_t								AddNode(CResource<CBaseControl> pNode);
		void								RemoveNode(CTreeNode* pNode);
		CControl<CTreeNode>					GetNode(size_t i);
		size_t								GetNumNodes() { return m_ahNodes.size(); };

		virtual void						AddVisibilityButton() {};

		virtual void						Selected();

		bool								IsExpanded() { return m_hExpandButton->IsExpanded(); };
		void								SetExpanded(bool bExpanded) { m_hExpandButton->SetExpanded(bExpanded); };

		void								SetIcon(const CMaterialHandle& hMaterial) { m_hIconMaterial = hMaterial; };
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
		tvector<CControl<CTreeNode>>		m_ahNodes;
		CControl<CTreeNode>					m_hParent;
		CControl<CTree>						m_hTree;
		CLabel*								m_pLabel;	// A temporary holder
		CControl<CLabel>					m_hLabel;

		CMaterialHandle						m_hIconMaterial;

		CControl<CPictureButton>			m_hVisibilityButton;
		CControl<CPictureButton>			m_hEditButton;

		bool								m_bDraggable;

		class CExpandButton : public CPictureButton
		{
		public:
											CExpandButton(const CMaterialHandle& hMaterial);

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

		CControl<CExpandButton>				m_hExpandButton;
	};

	class CTree : public CPanel, public IDroppable
	{
		DECLARE_CLASS(CTree, CPanel);

		friend class CTreeNode;

	public:
											CTree(const CMaterialHandle& hArrowMaterial = CMaterialHandle(), const CMaterialHandle& hEditMaterial = CMaterialHandle(), const CMaterialHandle& hVisibilityMaterial = CMaterialHandle());
		virtual								~CTree();

	public:
		virtual void						CreateControls(CResource<CBaseControl> pThis);

		virtual void						Layout();
		virtual void						Think();
		virtual void						Paint();
		virtual void						Paint(float x, float y);
		virtual void						Paint(float x, float y, float w, float h);

		virtual bool						MousePressed(int code, int mx, int my);
		virtual bool						MouseReleased(int iButton, int mx, int my);
		virtual bool						MouseDoubleClicked(int iButton, int mx, int my);

		virtual CControlHandle				AddControl(CResource<CBaseControl> pControl, bool bToTail = false);
		virtual void						RemoveControl(CBaseControl* pControl);

		void								ClearTree();

		size_t								AddNode(const tstring& sName);
		template <typename T>
		size_t								AddNode(const tstring& sName, T* pObject);
		size_t								AddNode(CResource<CBaseControl> pNode, size_t iPosition = ~0);
		void								RemoveNode(CTreeNode* pNode);
		CControl<CTreeNode>					GetNode(size_t i);

		virtual CControl<CTreeNode>			GetSelectedNode() { if (m_iSelected == ~0) return CControl<CTreeNode>(); return m_ahAllNodes[m_iSelected]; };
		virtual size_t						GetSelectedNodeId() { return m_iSelected; };
		virtual void						Unselect() { m_iSelected = ~0; }
		virtual void						SetSelectedNode(size_t iNode);
		virtual void						SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual void						SetConfirmedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual void						SetDroppedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		// IDroppable
		virtual const FRect					GetHoldingRect() { return GetAbsDimensions(); };

		virtual void						AddDraggable(IDraggable*) {};
		virtual void						SetDraggable(IDraggable*, bool bDelete = true);
		virtual IDraggable*					GetDraggable(int i) { return static_cast<IDraggable*>(m_hDragging.Get()); };
		virtual IDraggable*					GetCurrentDraggable() { return static_cast<IDraggable*>(m_hDragging.Get()); };

		// I already know.
		virtual void						SetGrabbale(bool bGrabbable) {};
		virtual bool						IsGrabbale() { return true; };

		virtual bool						CanDropHere(IDraggable*) { return true; };

		virtual bool						IsInfinite() { return true; };
		virtual bool						IsVisible() { return BaseClass::IsVisible(); };

	public:
		tvector<CControl<CTreeNode>>		m_ahNodes;
		tvector<CControl<CTreeNode>>		m_ahAllNodes;

		float								m_flCurrentHeight;
		float								m_flCurrentDepth;

		size_t								m_iHilighted;
		size_t								m_iSelected;

		CMaterialHandle						m_hArrowMaterial;
		CMaterialHandle						m_hVisibilityMaterial;
		CMaterialHandle						m_hEditMaterial;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;

		IEventListener::Callback			m_pfnConfirmedCallback;
		IEventListener*						m_pConfirmedListener;

		IEventListener::Callback			m_pfnDroppedCallback;
		IEventListener*						m_pDroppedListener;

		bool								m_bMouseDown;
		int									m_iMouseDownX;
		int									m_iMouseDownY;

		CControl<CTreeNode>					m_hDragging;
		int									m_iAcceptsDragType;
	};

	template <typename T>
	class CTreeNodeObject : public CTreeNode
	{
	public:
		CTreeNodeObject(T* pObject, CControlHandle hParent, CControlHandle hTree, const tstring& sName)
			: CTreeNode(hParent, hTree, sName, "sans-serif")
		{
			m_pObject = pObject;
		}

	public:
		typedef void (*EditFnCallback)(T*, const tstring& sArgs);

		virtual void LayoutNode()
		{
			CTreeNode::LayoutNode();

			float iHeight = m_hLabel->GetTextHeight();

			if (m_hVisibilityButton)
			{
				m_hVisibilityButton->SetPos(GetWidth()-iHeight-14, 0);
				m_hVisibilityButton->SetSize(iHeight, iHeight);
			}

			if (m_hEditButton)
			{
				m_hEditButton->SetPos(GetWidth()-iHeight*2-16, 0);
				m_hEditButton->SetSize(iHeight, iHeight);
			}

			m_hLabel->SetAlpha(m_pObject->IsVisible()?255:100);
		}

		virtual void AddVisibilityButton()
		{
			m_hVisibilityButton = AddControl(CreateControl(new CPictureButton("@", m_hTree->m_hVisibilityMaterial)));
			m_hVisibilityButton->SetClickedListener(this, Visibility);
		}

		virtual void AddEditButton(EditFnCallback pfnCallback)
		{
			m_hEditButton = AddControl(CreateControl(new CPictureButton("*", m_hTree->m_hEditMaterial)));
			m_hEditButton->SetClickedListener(this, Edit);
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

		m_hLabel->SetAlpha(m_pObject->IsVisible()?255:100);
	}

	template <typename T>
	inline void CTreeNodeObject<T>::EditCallback(const tstring& sArgs)
	{
		m_pfnCallback(m_pObject, sArgs);
	}

	template <typename T>
	inline size_t CTreeNode::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(CreateControl(new CTreeNodeObject<T>(pObject, m_hThis, m_hTree, sName)));
	}

	template <typename T>
	inline size_t CTree::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(CreateControl(new CTreeNodeObject<T>(pObject, CControlHandle(), m_hThis, sName)));
	}
};

#endif
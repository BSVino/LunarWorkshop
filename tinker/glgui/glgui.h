#ifndef TINKER_GLGUI_H
#define TINKER_GLGUI_H

#include <vector.h>
#include <geometry.h>
#include <tstring.h>
#include <tinker_memory.h>

// Not my favorite hack.
#define EVENT_CALLBACK(type, pfn) \
	void pfn##Callback(const tstring& sArgs); \
	static void pfn(glgui::IEventListener* obj, const tstring& sArgs) \
	{ \
		((type*)obj)->pfn##Callback(sArgs); \
	}

namespace glgui
{
	class CBaseControl;
	class CButton;
	class CCheckBox;
	class CDroppablePanel;
	class CLabel;
	class CMenu;
	class CMenuBar;
	class CPanel;
	class CMovablePanel;
	class CPictureButton;
	class CRootPanel;
	class CSlidingPanel;
	class CSlidingContainer;
	class CTextField;
	class CTree;
	class CTreeNode;

	extern Color g_clrPanel;
	extern Color g_clrBox;
	extern Color g_clrBoxHi;

	extern float g_flLayoutDefault;

	CResource<CBaseControl> CreateControl(CBaseControl* pControl);

	class IDroppable;

	// An object that can be grabbed and dragged around the screen.
	class IDraggable
	{
	public:
		typedef enum
		{
			DC_UNSPECIFIED	= 0,
		} DragClass_t;		// Where the hookers go to learn their trade.

		virtual					~IDraggable() {};

		virtual void			SetHoldingRect(const FRect&)=0;
		virtual FRect			GetHoldingRect()=0;

		virtual IDroppable*		GetDroppable()=0;
		virtual void			SetDroppable(IDroppable* pDroppable)=0;

		virtual void			Paint()=0;
		virtual void			Paint(float x, float y)=0;
		virtual void			Paint(float x, float y, float w, float h)=0;
		virtual void			Paint(float x, float y, float w, float h, bool bFloating)=0;

		virtual float			GetWidth()=0;
		virtual float			GetHeight()=0;

		virtual DragClass_t		GetClass()=0;
		virtual IDraggable*		MakeCopy()=0;
		virtual bool			IsDraggable() { return true; };
	};

	// A place where an IDraggable is allowed to be dropped.
	class IDroppable
	{
	public:
		virtual					~IDroppable() {};

		// Get the place where a droppable object should be.
		virtual const FRect		GetHoldingRect()=0;

		virtual void			AddDraggable(IDraggable*)=0;
		virtual void			SetDraggable(IDraggable*, bool bDelete = true)=0;
		virtual IDraggable*		GetDraggable(int i)=0;
		virtual IDraggable*		GetCurrentDraggable()=0;

		// I already know.
		virtual void			SetGrabbale(bool bGrabbable)=0;
		virtual bool			IsGrabbale()=0;

		virtual bool			CanDropHere(IDraggable*)=0;

		// Is this droppable a bottomless pit of draggables?
		virtual bool			IsInfinite()=0;

		virtual bool			IsVisible()=0;
	};

	class IEventListener
	{
	public:
		typedef void (*Callback)(IEventListener*, const tstring& sArgs);
	};
};

#endif

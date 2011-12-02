#ifndef SMAK_PICKER_H
#define SMAK_PICKER_H

#include "modelwindow_ui.h"

namespace glgui
{
	class CTree;
	class CTreeNode;
}

class CPicker : public CMovablePanel
{
public:
								CPicker(const tstring& sName, IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Delete() { delete this; };

public:
	virtual void				Layout();

	virtual void				Think();

	virtual void				PopulateTree() {};

	EVENT_CALLBACK(CPicker,		Selected);

	virtual void				NodeSelected(glgui::CTreeNode* pNode) {};

protected:
	virtual void				Open();
	virtual void				Close();

	glgui::IEventListener::Callback	m_pfnCallback;
	glgui::IEventListener*		m_pCallback;

	glgui::CTree*				m_pTree;

	bool						m_bPopulated;
};

class CMeshInstancePicker : public CPicker
{
public:
								CMeshInstancePicker(IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Delete() { delete this; };

public:
	virtual void				PopulateTree();
	virtual void				PopulateTreeNode(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode);

	virtual void				NodeSelected(glgui::CTreeNode* pNode);

	virtual CConversionMeshInstance*	GetPickedMeshInstance() { return m_pPickedMeshInstance; }

protected:
	CConversionMeshInstance*	m_pPickedMeshInstance;
};

#endif

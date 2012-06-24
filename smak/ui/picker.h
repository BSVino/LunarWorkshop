#ifndef SMAK_PICKER_H
#define SMAK_PICKER_H

#include <glgui/movablepanel.h>

#include "smakwindow_ui.h"

class CPicker : public glgui::CMovablePanel
{
public:
								CPicker(const tstring& sName, IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Layout();

	virtual void				Think();

	virtual void				PopulateTree() {};

	EVENT_CALLBACK(CPicker,		Selected);

	virtual void				NodeSelected(glgui::CTreeNode* pNode) {};

protected:
	virtual void				Open();

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
	virtual void				PopulateTree();
	virtual void				PopulateTreeNode(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode);

	virtual void				NodeSelected(glgui::CTreeNode* pNode);

	virtual CConversionMeshInstance*	GetPickedMeshInstance() { return m_pPickedMeshInstance; }

protected:
	CConversionMeshInstance*	m_pPickedMeshInstance;
};

#endif

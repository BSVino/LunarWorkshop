#ifndef SMAK_SCENETREE_H
#define SMAK_SCENETREE_H

#include <glgui/movablepanel.h>

#include "smakwindow_ui.h"

class CSceneTreePanel : public glgui::CMovablePanel
{
public:
									CSceneTreePanel(CConversionScene* pScene);
									~CSceneTreePanel();

public:
	void							Layout();
	void							Paint(float x, float y, float w, float h);

	void							UpdateTree();
	void							AddAllToTree();
	void							AddNodeToTree(glgui::CTreeNode* pTreeNode, CConversionSceneNode* pNode);

	void							OpenMaterialEditor(CConversionMaterial* pMaterial);

	size_t							GetLastSelectedMaterial() { return m_iLastSelectedMaterial; }

	EVENT_CALLBACK(CSceneTreePanel, Selected);

	static void						Open(CConversionScene* pScene);
	static CSceneTreePanel*			Get();

public:
	CConversionScene*				m_pScene;

	glgui::CTree*					m_pTree;

	class CMaterialEditor*			m_pMaterialEditor;

	size_t							m_iLastSelectedMaterial;

	static CSceneTreePanel*			s_pSceneTreePanel;
};

void OpenMaterialEditor(CConversionMaterial* pMaterial, const tstring& sArgs);

class CMaterialEditor : public glgui::CMovablePanel
{
public:
									CMaterialEditor(CConversionMaterial* pMaterial, CSceneTreePanel* pSceneTree);

public:
	void							SetupSelector(glgui::CScrollSelector<float>* pSelector, float flMaxValue);

	void							Layout();

	EVENT_CALLBACK(CMaterialEditor, ChooseDiffuse);
	EVENT_CALLBACK(CMaterialEditor, ChooseNormal);
	EVENT_CALLBACK(CMaterialEditor, OpenDiffuse);
	EVENT_CALLBACK(CMaterialEditor, OpenNormal);
	EVENT_CALLBACK(CMaterialEditor, RemoveDiffuse);
	EVENT_CALLBACK(CMaterialEditor, RemoveNormal);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientRed);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientGreen);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientBlue);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseRed);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseGreen);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseBlue);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularRed);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularGreen);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularBlue);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveRed);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveGreen);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveBlue);
	EVENT_CALLBACK(CMaterialEditor, SetShininess);

protected:
	CConversionMaterial*			m_pMaterial;
	CSceneTreePanel*				m_pSceneTree;
	CConversionScene*				m_pScene;
	size_t							m_iMaterial;

	glgui::CLabel*					m_pDiffuseLabel;
	glgui::CButton*					m_pDiffuseFile;
	glgui::CButton*					m_pDiffuseRemove;

	glgui::CLabel*					m_pNormalLabel;
	glgui::CButton*					m_pNormalFile;
	glgui::CButton*					m_pNormalRemove;

	glgui::CLabel*					m_pAmbientLabel;
	glgui::CScrollSelector<float>*	m_pAmbientRedSelector;
	glgui::CScrollSelector<float>*	m_pAmbientGreenSelector;
	glgui::CScrollSelector<float>*	m_pAmbientBlueSelector;

	glgui::CLabel*					m_pDiffuseSelectorLabel;
	glgui::CScrollSelector<float>*	m_pDiffuseRedSelector;
	glgui::CScrollSelector<float>*	m_pDiffuseGreenSelector;
	glgui::CScrollSelector<float>*	m_pDiffuseBlueSelector;

	glgui::CLabel*					m_pSpecularLabel;
	glgui::CScrollSelector<float>*	m_pSpecularRedSelector;
	glgui::CScrollSelector<float>*	m_pSpecularGreenSelector;
	glgui::CScrollSelector<float>*	m_pSpecularBlueSelector;

	glgui::CLabel*					m_pEmissiveLabel;
	glgui::CScrollSelector<float>*	m_pEmissiveRedSelector;
	glgui::CScrollSelector<float>*	m_pEmissiveGreenSelector;
	glgui::CScrollSelector<float>*	m_pEmissiveBlueSelector;

	glgui::CLabel*					m_pShininessLabel;
	glgui::CScrollSelector<float>*	m_pShininessSelector;
};

inline CSceneTreePanel* SceneTree()
{
	return CSceneTreePanel::Get();
}

#endif
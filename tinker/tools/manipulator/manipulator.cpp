#include "manipulator.h"

#include <geometry.h>

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <glgui/rootpanel.h>
#include <glgui/picturebutton.h>
#include <textures/materiallibrary.h>
#include <renderer/renderingcontext.h>

#include "IGizmo.h"

CManipulatorTool::CManipulatorTool()
{
	m_bActive = false;
	m_pListener = nullptr;
	m_bTransforming = false;
	m_eTransform = MT_TRANSLATE;

	m_pTranslateGizmo = CreateMoveGizmo();
	m_pRotateGizmo = CreateRotateGizmo();
	m_pScaleGizmo = CreateScaleGizmo();

	m_pTranslateButton = new glgui::CPictureButton("Translate", CMaterialLibrary::AddMaterial("editor/translate.mat"));
	glgui::CRootPanel::Get()->AddControl(m_pTranslateButton, true);
	m_pTranslateButton->Layout_AlignBottom(nullptr, 20);
	m_pTranslateButton->Layout_ColumnFixed(3, 0, m_pTranslateButton->GetWidth());
	m_pTranslateButton->SetClickedListener(this, TranslateMode);
	m_pTranslateButton->SetTooltip("Translate");
	m_pTranslateButton->SetVisible(false);

	m_pRotateButton = new glgui::CPictureButton("Rotate", CMaterialLibrary::AddMaterial("editor/rotate.mat"));
	glgui::CRootPanel::Get()->AddControl(m_pRotateButton, true);
	m_pRotateButton->Layout_AlignBottom(nullptr, 20);
	m_pRotateButton->Layout_ColumnFixed(3, 1, m_pRotateButton->GetWidth());
	m_pRotateButton->SetClickedListener(this, RotateMode);
	m_pRotateButton->SetTooltip("Rotate");
	m_pRotateButton->SetVisible(false);

	m_pScaleButton = new glgui::CPictureButton("Scale", CMaterialLibrary::AddMaterial("editor/scale.mat"));
	glgui::CRootPanel::Get()->AddControl(m_pScaleButton, true);
	m_pScaleButton->Layout_AlignBottom(nullptr, 20);
	m_pScaleButton->Layout_ColumnFixed(3, 2, m_pScaleButton->GetWidth());
	m_pScaleButton->SetClickedListener(this, ScaleMode);
	m_pScaleButton->SetTooltip("Scale");
	m_pScaleButton->SetVisible(false);
}

void CManipulatorTool::Activate(IManipulatorListener* pListener, const TRS& trs, const tstring& sArguments)
{
	m_bActive = true;
	m_pListener = pListener;
	m_trsTransform = trs;
	m_sListenerArguments = sArguments;
	m_pTranslateButton->SetVisible(true);
	m_pRotateButton->SetVisible(true);
	m_pScaleButton->SetVisible(true);

	m_mTransform = trs.GetMatrix4x4();

	s_pManipulatorTool->m_pTranslateGizmo->SetEditMatrix(m_mTransform);
	s_pManipulatorTool->m_pRotateGizmo->SetEditMatrix(m_mTransform);
	s_pManipulatorTool->m_pScaleGizmo->SetEditMatrix(m_mTransform);
}

void CManipulatorTool::Deactivate()
{
	m_bActive = false;
	m_pTranslateButton->SetVisible(false);
	m_pRotateButton->SetVisible(false);
	m_pScaleButton->SetVisible(false);
}

bool CManipulatorTool::IsTransforming()
{
	if (!IsActive())
		return false;

	if (s_pManipulatorTool->m_eTransform == MT_TRANSLATE)
		return m_pTranslateGizmo->IsTransforming();
	else if (s_pManipulatorTool->m_eTransform == MT_ROTATE)
		return m_pRotateGizmo->IsTransforming();
	else if (s_pManipulatorTool->m_eTransform == MT_SCALE)
		return m_pScaleGizmo->IsTransforming();

	return false;
}

void CManipulatorTool::SetTransfromType(TransformType eTransform)
{
	m_eTransform = eTransform;
}

bool CManipulatorTool::MouseInput(int iButton, tinker_mouse_state_t iState, int mx, int my)
{
	if (!s_pManipulatorTool)
		return false;

	if (!s_pManipulatorTool->IsActive())
		return false;

	if (iState == TINKER_MOUSE_PRESSED)
	{
		if (s_pManipulatorTool->m_eTransform == MT_TRANSLATE)
			s_pManipulatorTool->m_bHasMouse = s_pManipulatorTool->m_pTranslateGizmo->OnMouseDown(mx, my);
		else if (s_pManipulatorTool->m_eTransform == MT_ROTATE)
			s_pManipulatorTool->m_bHasMouse = s_pManipulatorTool->m_pRotateGizmo->OnMouseDown(mx, my);
		else if (s_pManipulatorTool->m_eTransform == MT_SCALE)
			s_pManipulatorTool->m_bHasMouse = s_pManipulatorTool->m_pScaleGizmo->OnMouseDown(mx, my);

		return s_pManipulatorTool->m_bHasMouse;
	}
	else
	{
		if (s_pManipulatorTool->m_eTransform == MT_TRANSLATE)
			s_pManipulatorTool->m_pTranslateGizmo->OnMouseUp(mx, my);
		else if (s_pManipulatorTool->m_eTransform == MT_ROTATE)
			s_pManipulatorTool->m_pRotateGizmo->OnMouseUp(mx, my);
		else if (s_pManipulatorTool->m_eTransform == MT_SCALE)
			s_pManipulatorTool->m_pScaleGizmo->OnMouseUp(mx, my);

		Matrix4x4 mTransform = s_pManipulatorTool->m_mTransform;
		s_pManipulatorTool->m_trsTransform.m_vecTranslation = mTransform.GetTranslation();
		s_pManipulatorTool->m_trsTransform.m_vecScaling = mTransform.GetScale();

		mTransform.SetForwardVector(mTransform.GetForwardVector()/s_pManipulatorTool->m_trsTransform.m_vecScaling.x);
		mTransform.SetUpVector(mTransform.GetUpVector()/s_pManipulatorTool->m_trsTransform.m_vecScaling.y);
		mTransform.SetRightVector(mTransform.GetRightVector()/s_pManipulatorTool->m_trsTransform.m_vecScaling.z);

		s_pManipulatorTool->m_trsTransform.m_angRotation = mTransform.GetAngles();

		if (s_pManipulatorTool->m_pListener)
		{
			if (Application()->IsShiftDown())
				s_pManipulatorTool->m_pListener->DuplicateMove(s_pManipulatorTool->m_sListenerArguments);
			else
				s_pManipulatorTool->m_pListener->ManipulatorUpdated(s_pManipulatorTool->m_sListenerArguments);
		}

		return s_pManipulatorTool->m_bHasMouse;
	}
}

void CManipulatorTool::MouseMoved(int mx, int my)
{
	if (!s_pManipulatorTool)
		return;

	if (!s_pManipulatorTool->IsActive())
		return;

	if (s_pManipulatorTool->m_eTransform == MT_TRANSLATE)
		s_pManipulatorTool->m_pTranslateGizmo->OnMouseMove(mx, my);
	else if (s_pManipulatorTool->m_eTransform == MT_ROTATE)
		s_pManipulatorTool->m_pRotateGizmo->OnMouseMove(mx, my);
	else if (s_pManipulatorTool->m_eTransform == MT_SCALE)
		s_pManipulatorTool->m_pScaleGizmo->OnMouseMove(mx, my);
}

void CManipulatorTool::Render(CRenderer* pRenderer)
{
	if (!s_pManipulatorTool)
		return;

	if (!s_pManipulatorTool->IsActive())
		return;

	s_pManipulatorTool->m_pRenderer = pRenderer;

	const double* pflModelView = pRenderer->GetModelView();
	const double* pflProjection = pRenderer->GetProjection();
	const int* piViewport = pRenderer->GetViewport();

	float aflModelView[16];
	float aflProjection[16];
	for (size_t i = 0; i < 16; i++)
	{
		aflModelView[i] = (float)pflModelView[i];
		aflProjection[i] = (float)pflProjection[i];
	}

	s_pManipulatorTool->m_pTranslateGizmo->SetCameraMatrix(aflModelView, aflProjection);
	s_pManipulatorTool->m_pRotateGizmo->SetCameraMatrix(aflModelView, aflProjection);
	s_pManipulatorTool->m_pScaleGizmo->SetCameraMatrix(aflModelView, aflProjection);

	s_pManipulatorTool->m_pTranslateGizmo->SetScreenDimension(piViewport[2], piViewport[3]);
	s_pManipulatorTool->m_pRotateGizmo->SetScreenDimension(piViewport[2], piViewport[3]);
	s_pManipulatorTool->m_pScaleGizmo->SetScreenDimension(piViewport[2], piViewport[3]);

	if (s_pManipulatorTool->m_eTransform == MT_TRANSLATE)
		s_pManipulatorTool->m_pTranslateGizmo->Draw();
	else if (s_pManipulatorTool->m_eTransform == MT_ROTATE)
		s_pManipulatorTool->m_pRotateGizmo->Draw();
	else if (s_pManipulatorTool->m_eTransform == MT_SCALE)
		s_pManipulatorTool->m_pScaleGizmo->Draw();
}

Matrix4x4 CManipulatorTool::GetTransform(bool bRotation, bool bScaling)
{
	if (m_bTransforming)
		return GetNewTRS().GetMatrix4x4(bRotation, bScaling);

	return m_trsTransform.GetMatrix4x4(bRotation, bScaling);
}

void CManipulatorTool::SetTRS(const TRS& trs)
{
	m_trsTransform = trs;
	m_mTransform = trs.GetMatrix4x4();
}

TRS CManipulatorTool::GetNewTRS()
{
	Matrix4x4 mTransform = m_mTransform;
	m_trsTransform.m_vecTranslation = mTransform.GetTranslation();
	m_trsTransform.m_vecScaling = mTransform.GetScale();

	mTransform.SetForwardVector(mTransform.GetForwardVector()/m_trsTransform.m_vecScaling.x);
	mTransform.SetUpVector(mTransform.GetUpVector()/m_trsTransform.m_vecScaling.y);
	mTransform.SetRightVector(mTransform.GetRightVector()/m_trsTransform.m_vecScaling.z);

	m_trsTransform.m_angRotation = mTransform.GetAngles();

	return m_trsTransform;
}

void CManipulatorTool::TranslateModeCallback(const tstring& sArgs)
{
	SetTransfromType(MT_TRANSLATE);
}

void CManipulatorTool::RotateModeCallback(const tstring& sArgs)
{
	SetTransfromType(MT_ROTATE);
}

void CManipulatorTool::ScaleModeCallback(const tstring& sArgs)
{
	SetTransfromType(MT_SCALE);
}

CRenderer* CManipulatorTool::GetRenderer()
{
	return m_pRenderer;
}

CManipulatorTool* CManipulatorTool::s_pManipulatorTool = nullptr;

CManipulatorTool* CManipulatorTool::Get()
{
	if (!s_pManipulatorTool)
		s_pManipulatorTool = new CManipulatorTool();

	return s_pManipulatorTool;
}

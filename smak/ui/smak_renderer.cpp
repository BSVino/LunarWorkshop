#include "smak_renderer.h"

#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>

#include "smakwindow.h"
#include "models/models.h"

CSMAKRenderer::CSMAKRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_bDrawBackground = true;
}

void CSMAKRenderer::Render()
{
	if (SMAKWindow()->IsRenderingUV())
	{
		m_bRenderOrthographic = true;

		m_flCameraOrthoHeight = SMAKWindow()->GetCameraUVZoom();
		m_vecCameraPosition.x = SMAKWindow()->GetCameraUVX();
		m_vecCameraPosition.y = SMAKWindow()->GetCameraUVY();
	}
	else
	{
		m_bRenderOrthographic = false;

		float flSceneSize = SMAKWindow()->GetScene()->m_oExtends.Size().Length()/2;
		if (flSceneSize < 150)
			flSceneSize = 150;

		m_flCameraFOV = 44;
		m_flCameraNear = 1;
		m_flCameraFar = SMAKWindow()->GetCameraDistance() + flSceneSize;

		Vector vecSceneCenter = SMAKWindow()->GetScene()->m_oExtends.Center();

		Vector vecCameraVector = AngleVector(EAngle(SMAKWindow()->GetCameraPitch(), SMAKWindow()->GetCameraYaw(), 0)) * -SMAKWindow()->GetCameraDistance() + vecSceneCenter;

		m_vecCameraPosition = vecCameraVector;
		m_vecCameraDirection = (vecSceneCenter - vecCameraVector).Normalized();
		m_vecCameraUp = Vector(0, 1, 0);
	}

	PreRender();

	{
		CRenderingContext c(this);
		ModifyContext(&c);
		SetupFrame(&c);
		StartRendering(&c);

		if (SMAKWindow()->IsRenderingUV())
			RenderUV();
		else
			Render3D();

		FinishRendering(&c);
		FinishFrame(&c);
	}

	PostRender();
}

void CSMAKRenderer::DrawBackground(CRenderingContext* r)
{
	CRenderingContext c;
	c.SetWinding(true);
	c.SetDepthTest(false);
	c.SetBackCulling(false);

	c.UseFrameBuffer(&m_oSceneBuffer);

	c.UseProgram("background");

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.EndRenderVertexArray(6);
}

void CSMAKRenderer::Render3D()
{
	// Reposition the light source.
	Vector vecLightDirection = AngleVector(EAngle(SMAKWindow()->GetLightPitch(), SMAKWindow()->GetLightYaw(), 0));

	m_vecLightPosition = vecLightDirection * -SMAKWindow()->GetCameraDistance()/2;

	RenderGround();

	RenderObjects();

#if 0
	// Render light source on top of objects, since it doesn't use the depth buffer.
	RenderLightSource();

	if (m_aDebugLines.size())
	{
#ifdef OPENGL2
		glLineWidth(1);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			for (size_t i = 0; i < m_aDebugLines.size(); i++)
			{
				glColor3ubv(m_aDebugLines[i].clrLine);
				glVertex3fv(m_aDebugLines[i].vecStart);
				glVertex3fv(m_aDebugLines[i].vecEnd);
			}
		glEnd();
		glBegin(GL_POINTS);
			glColor3f(0.6f, 0.6f, 0.6f);
			for (size_t i = 0; i < m_aDebugLines.size(); i++)
			{
				glVertex3fv(m_aDebugLines[i].vecStart);
				glVertex3fv(m_aDebugLines[i].vecEnd);
			}
		glEnd();
#endif
	}

#ifdef RAYTRACE_DEBUG
	static raytrace::CRaytracer* pTracer = NULL;
	Vector vecStart(0.55841917f, 0.28102291f, 2.4405572f);
	Vector vecDirection(-0.093336411f, 0.99130136f, -0.092789143f);
	if (!pTracer && GetScene()->GetNumScenes() && !m_bLoadingFile)
	{
		pTracer = new raytrace::CRaytracer(GetScene());
		pTracer->AddMeshInstance(GetScene()->GetScene(0)->GetChild(2)->GetMeshInstance(0));
		pTracer->BuildTree();
		AddDebugLine(vecStart, vecStart+vecDirection*10);
	}
	if (pTracer)
		pTracer->Raytrace(Ray(vecStart, vecDirection));
#endif

#ifdef OPENGL2
	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
#endif
#endif
}

void CSMAKRenderer::RenderGround()
{
	CRenderingContext c(this, true);

	c.UseProgram("model");
	c.Translate(SMAKWindow()->GetScene()->m_oExtends.Center());
	c.SetUniform("flAlpha", 1.0f);
	c.SetUniform("flRimLight", 0.0f);
	c.SetUniform("bDiffuse", false);

	int i;

	Color clrBorderLineBright(0.7f, 0.7f, 0.7f);
	Color clrBorderLineDarker(0.6f, 0.6f, 0.6f);
	Color clrInsideLineBright(0.5f, 0.5f, 0.5f);
	Color clrInsideLineDarker(0.4f, 0.4f, 0.4f);

	for (i = 0; i < 20; i++)
	{
		Vector vecStartX(-100, 0, -100);
		Vector vecEndX(-100, 0, 100);
		Vector vecStartZ(-100, 0, -100);
		Vector vecEndZ(100, 0, -100);

		for (int j = 0; j <= 20; j++)
		{
			c.BeginRenderLines();

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineBright);
				else
					c.Color(clrInsideLineBright);

				c.Vertex(vecStartX);

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineDarker);
				else
					c.Color(clrInsideLineDarker);

				if (j == 10)
					c.Vertex(Vector(0, 0, 0));
				else
					c.Vertex(vecEndX);

			c.EndRender();

			if (j == 10)
			{
				c.BeginRenderLines();
					c.Color(Color(0.9f, 0.2f, 0.2f));
					c.Vertex(Vector(0, 0, 0));
					c.Vertex(Vector(100, 0, 0));
				c.EndRender();
			}

			c.BeginRenderLines();

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineBright);
				else
					c.Color(clrInsideLineBright);

				c.Vertex(vecStartZ);

				if (j == 0 || j == 20 || j == 10)
					c.Color(clrBorderLineDarker);
				else
					c.Color(clrInsideLineDarker);

				if (j == 10)
					c.Vertex(Vector(0, 0, 0));
				else
					c.Vertex(vecEndZ);

			c.EndRender();

			if (j == 10)
			{
				c.BeginRenderLines();
					c.Color(Color(0.2f, 0.2f, 0.7f));
					c.Vertex(Vector(0, 0, 0));
					c.Vertex(Vector(0, 0, 100));
				c.EndRender();
			}

			vecStartX.x += 10;
			vecEndX.x += 10;
			vecStartZ.z += 10;
			vecEndZ.z += 10;
		}
	}
}

void CSMAKRenderer::RenderObjects()
{
	CRenderingContext c(this, true);

#ifdef OPENGL2
	if (m_bDisplayLight)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
#endif

	for (size_t i = 0; i < SMAKWindow()->GetScene()->GetNumScenes(); i++)
		RenderSceneNode(SMAKWindow()->GetScene()->GetScene(i));
}

void CSMAKRenderer::RenderSceneNode(CConversionSceneNode* pNode)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	CRenderingContext c(this, true);
	c.Transform(pNode->m_mTransformations);

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pNode->GetChild(i));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pNode->GetMeshInstance(m));
}

void CSMAKRenderer::RenderMeshInstance(CConversionMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->IsVisible())
		return;

	CModel* pModel = CModelLibrary::GetModel(pMeshInstance->m_iMesh);

	if (!pModel)
		return;

	CRenderingContext c(this, true);

	for (size_t i = 0; i < pModel->m_asMaterialStubs.size(); i++)
	{
		auto pMaterialMap = pMeshInstance->GetMappedMaterial(i);

		if (!pMaterialMap)
			continue;

		if (!pMaterialMap->IsVisible())
			continue;

		size_t iMaterial = pMaterialMap->m_iMaterial;

		if (!SMAKWindow()->GetScene()->GetMaterial(iMaterial)->IsVisible())
			continue;

		if (!pModel->m_aiVertexBufferSizes[i])
			continue;

		c.UseMaterial(SMAKWindow()->GetMaterials()[iMaterial]);

		if (!c.GetActiveShader())
		{
			c.UseProgram("model");

			c.SetUniform("flAlpha", 1.0f);
			c.SetUniform("vecColor", Color(255, 255, 255));
			c.SetUniform("bDiffuse", false);
		}

		c.SetUniform("flRimLight", 0.05f);

		c.BeginRenderVertexArray(pModel->m_aiVertexBuffers[i]);
		c.SetPositionBuffer(pModel->PositionOffset(), pModel->Stride());
		c.SetNormalsBuffer(pModel->NormalsOffset(), pModel->Stride());
		c.SetTexCoordBuffer(pModel->TexCoordOffset(), pModel->Stride());
		c.EndRenderVertexArray(pModel->m_aiVertexBufferSizes[i]);
	}

#if 0
	// It uses this color if the texture is missing.
	float flMaterialColor[] = {0.7f, 0.7f, 0.7f, 1.0f};

#ifdef OPENGL2
	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);
#endif

	bool bMultiTexture = false;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	size_t iFaces = pMesh->GetNumFaces();
	for (size_t j = 0; j < iFaces; j++)
	{
//		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		CConversionMaterialMap* pMappedMaterial = pMeshInstance->GetMappedMaterial(pFace->m);

		if (pFace->m != ~0 && pMappedMaterial)
		{
			if (!pMappedMaterial->IsVisible())
				continue;

			CConversionMaterial* pMaterial = m_Scene.GetMaterial(pMappedMaterial->m_iMaterial);
			if (pMaterial && !pMaterial->IsVisible())
				continue;
		}

		if (!m_bDisplayWireframe)
		{
			bool bTexture = m_bDisplayTexture;
			bool bNormal = m_bDisplayNormal;
			bool bNormal2 = m_bDisplayNormal;
			g_bNormalMap = m_bDisplayNormal;
			bool bAO = m_bDisplayAO;
			bool bCAO = m_bDisplayColorAO;

			if (!m_Scene.DoesFaceHaveValidMaterial(pFace, pMeshInstance) || pMappedMaterial->m_iMaterial >= m_aoMaterials.size())
			{
#ifdef OPENGL2
				glBindTexture(GL_TEXTURE_2D, 0);
#endif
				bTexture = false;
				bNormal = false;
				bNormal2 = false;
				g_bNormalMap = false;
				bAO = false;
				bCAO = false;
			}
			else
			{
				CMaterial* pMaterial = &m_aoMaterials[pMappedMaterial->m_iMaterial];

				if (!pMaterial->m_iBase)
					bTexture = false;
				if (!pMaterial->m_iNormal)
					bNormal = false;
				if (!pMaterial->m_iNormal2)
					bNormal2 = false;
				if (!bNormal && !bNormal2)
					g_bNormalMap = false;
				if (!pMaterial->m_iAO)
					bAO = false;
				if (!pMaterial->m_iColorAO)
					bCAO = false;

#ifdef OPENGL2
				if (GLEW_VERSION_1_3)
				{
					bMultiTexture = true;

					glActiveTexture(GL_TEXTURE0);
					if (m_bDisplayTexture)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE1);
					if (bNormal)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE2);
					if (bNormal2)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal2);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE3);
					if (m_bDisplayAO)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iAO);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE4);
					if (m_bDisplayColorAO)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iColorAO);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}
				}
				else
				{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
				}
#endif
			}

#ifdef OPENGL2
			if (m_Scene.DoesFaceHaveValidMaterial(pFace, pMeshInstance))
			{
				CConversionMaterial* pMaterial = m_Scene.GetMaterial(pMappedMaterial->m_iMaterial);
				glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
				glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
				glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
				glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
				glColor4fv(pMaterial->m_vecDiffuse);
			}

			glUseProgram((GLuint)m_iShaderProgram);

			GLuint bLighting = glGetUniformLocation((GLuint)m_iShaderProgram, "bLighting");
			GLuint bDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "bDiffuseTexture");
			GLuint bNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormalMap");
			GLuint bNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormal2Map");
			GLuint bAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bAOMap");
			GLuint bCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bCAOMap");

			GLuint iDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "iDiffuseTexture");
			GLuint iNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormalMap");
			GLuint iNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormal2Map");
			GLuint iAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iAOMap");
			GLuint iCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iCAOMap");

			GLuint bShadeBottoms = glGetUniformLocation((GLuint)m_iShaderProgram, "bShadeBottoms");

			g_iTangentAttrib = glGetAttribLocation((GLuint)m_iShaderProgram, "vecTangent");
			g_iBitangentAttrib = glGetAttribLocation((GLuint)m_iShaderProgram, "vecBitangent");

			glUniform1i(iDiffuseTexture, 0);
			glUniform1i(iNormalMap, 1);
			glUniform1i(iNormal2Map, 2);
			glUniform1i(iAOMap, 3);
			glUniform1i(iCAOMap, 4);

			glUniform1i(bLighting, m_bDisplayLight);
			glUniform1i(bDiffuseTexture, bTexture);
			glUniform1i(bNormalMap, bNormal);
			glUniform1i(bNormal2Map, bNormal2);
			glUniform1i(bAOMap, bAO);
			glUniform1i(bCAOMap, bCAO);

			glUniform1i(bShadeBottoms, true);

			gluTessBeginPolygon(m_pTesselator, pMesh);
			gluTessBeginContour(m_pTesselator);

			for (k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				GLdouble afCoords[3] = { vecVertex.x, vecVertex.y, vecVertex.z };
				gluTessVertex(m_pTesselator, afCoords, pVertex);
			}

			gluTessEndContour(m_pTesselator);
			gluTessEndPolygon(m_pTesselator);

			glUseProgram(0);
#endif
		}

#ifdef OPENGL2
		if (m_bDisplayWireframe)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_LINE_STRIP);
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(0)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(1)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(1)->v));
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(2)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(2)->v));
			glEnd();
			for (k = 0; k < pFace->GetNumVertices()-2; k++)
			{
				glBegin(GL_LINES);
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+1)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v));
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+2)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+2)->v));
				glEnd();
			}
			glBegin(GL_LINES);
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+1)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v));
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(0)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
			glEnd();
		}
#endif

#if 0
		glDisable(GL_LIGHTING);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)0);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			Vector vecVertex = pMesh->GetVertex(pVertex->v);
			Vector vecNormal = pMesh->GetNormal(pVertex->vn);
			Vector vecTangent = pMesh->GetTangent(pVertex->vt);
			Vector vecBitangent = pMesh->GetBitangent(pVertex->vb);
			//vecNormal = Vector(pVertex->m_mInverseTBN.GetColumn(2));
			//vecTangent = Vector(pVertex->m_mInverseTBN.GetColumn(0));
			//vecBitangent = Vector(pVertex->m_mInverseTBN.GetColumn(1));

			glColor3f(0.2f, 0.2f, 0.8f);

			glBegin(GL_LINES);
			glNormal3fv(vecNormal);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecNormal);
			glEnd();

			glColor3f(0.8f, 0.2f, 0.2f);

			glBegin(GL_LINES);
			glNormal3fv(vecTangent);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecTangent);
			glEnd();

			glColor3f(0.2f, 0.8f, 0.2f);

			glBegin(GL_LINES);
			glNormal3fv(vecBitangent);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecBitangent);
			glEnd();
		}
#endif
	}
#endif
}

void CSMAKRenderer::RenderUV()
{
#ifdef OPENGL2
	glViewport(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	// Switch GL to 2d drawing mode.

	float flRatio = (float)m_iWindowHeight / (float)m_iWindowWidth;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -flRatio, flRatio, -1, 1);

	glScalef(m_flCameraUVZoom, m_flCameraUVZoom, 0);
	glTranslatef(m_flCameraUVX, m_flCameraUVY, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	GLfloat flLightPosition[4];
	flLightPosition[0] = m_vecLightPositionUV.x;
	flLightPosition[1] = m_vecLightPositionUV.y;
	flLightPosition[2] = 1.0f;
	flLightPosition[3] = 0;

	// Tell GL new light source position.
    glLightfv(GL_LIGHT0, GL_POSITION, flLightPosition);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_CULL_FACE);

	glShadeModel(GL_FLAT);

	bool bMultiTexture = false;

	CMaterial* pMaterial = NULL;
	if (m_aoMaterials.size())
		pMaterial = &m_aoMaterials[0];

	bool bTexture = false;
	bool bNormal = false;
	bool bNormal2 = false;
	bool bAO = false;
	bool bCAO = false;

	if (!pMaterial)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (GLEW_VERSION_1_3)
	{
		bMultiTexture = true;

		glActiveTexture(GL_TEXTURE0);
		if (m_bDisplayTexture && pMaterial->m_iBase)
		{
			bTexture = true;
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE1);
		if (m_bDisplayNormal && pMaterial->m_iNormal)
		{
			bNormal = true;
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE2);
		if (m_bDisplayNormal && pMaterial->m_iNormal2)
		{
			bNormal2 = true;
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal2);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE3);
		if (m_bDisplayAO && pMaterial->m_iAO)
		{
			bAO = true;
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iAO);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE4);
		if (m_bDisplayColorAO && pMaterial->m_iColorAO)
		{
			bCAO = true;
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iColorAO);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}

	if (!pMaterial)
		glColor3f(0.8f, 0.8f, 0.8f);
	else if (!pMaterial->m_iBase && !(m_bDisplayNormal || m_bDisplayAO || m_bDisplayColorAO))
		glColor3f(0.0f, 0.0f, 0.0f);
	else if (m_bDisplayTexture || m_bDisplayNormal || m_bDisplayAO || m_bDisplayColorAO)
	{
		CConversionMaterial* pConversionMaterial = m_Scene.GetMaterial(0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, pConversionMaterial->m_vecAmbient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, pConversionMaterial->m_vecDiffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, pConversionMaterial->m_vecSpecular);
		glMaterialfv(GL_FRONT, GL_EMISSION, pConversionMaterial->m_vecEmissive);
		glMaterialf(GL_FRONT, GL_SHININESS, pConversionMaterial->m_flShininess);
		glColor4fv(pConversionMaterial->m_vecDiffuse);
	}
	else
		glColor3f(0.0f, 0.0f, 0.0f);


	glUseProgram((GLuint)m_iShaderProgram);

	GLuint bLighting = glGetUniformLocation((GLuint)m_iShaderProgram, "bLighting");
	GLuint bDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "bDiffuseTexture");
	GLuint bNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormalMap");
	GLuint bNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormal2Map");
	GLuint bAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bAOMap");
	GLuint bCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bCAOMap");

	GLuint iDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "iDiffuseTexture");
	GLuint iNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormalMap");
	GLuint iNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormal2Map");
	GLuint iAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iAOMap");
	GLuint iCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iCAOMap");

	GLuint bShadeBottoms = glGetUniformLocation((GLuint)m_iShaderProgram, "bShadeBottoms");

	GLuint iTangent = glGetAttribLocation((GLuint)m_iShaderProgram, "vecTangent");
	GLuint iBitangent = glGetAttribLocation((GLuint)m_iShaderProgram, "vecBitangent");

	glUniform1i(iDiffuseTexture, 0);
	glUniform1i(iNormalMap, 1);
	glUniform1i(iNormal2Map, 2);
	glUniform1i(iAOMap, 3);
	glUniform1i(iCAOMap, 4);

	glUniform1i(bLighting, m_bDisplayLight);
	glUniform1i(bDiffuseTexture, bTexture);
	glUniform1i(bNormalMap, bNormal);
	glUniform1i(bNormal2Map, bNormal2);
	glUniform1i(bAOMap, bAO);
	glUniform1i(bCAOMap, bCAO);

	glUniform1i(bShadeBottoms, bNormal||bNormal2);

	Vector vecUV;

	glBegin(GL_QUADS);

		vecUV = Vector(0.0f, 1.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertexAttrib3fv(iTangent, Vector(0.8165f, 0.4082f, 0.4082f));
		glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, -0.7071f));
		glNormal3f(-0.5574f, 0.5574f, 0.5574f);
		glVertex2f(-0.5f, 0.5f);

		vecUV = Vector(1.0f, 1.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertexAttrib3fv(iTangent, Vector(0.8165f, -0.4082f, -0.4082f));
		glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
		glNormal3f(0.5574f, 0.5574f, 0.5574f);
		glVertex2f(0.5f, 0.5f);

		vecUV = Vector(1.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertexAttrib3fv(iTangent, Vector(0.8165f, 0.4082f, -0.4082f));
		glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
		glNormal3f(0.5574f, -0.5574f, 0.5574f);
		glVertex2f(0.5f, -0.5f);

		vecUV = Vector(0.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertexAttrib3fv(iTangent, Vector(0.8165f, -0.4082f, 0.4082f));
		glVertexAttrib3fv(iBitangent, Vector(0.0f, 0.7071f, 0.7071f));
		glNormal3f(-0.5574f, -0.5574f, 0.5574f);
		glVertex2f(-0.5f, -0.5f);

	glEnd();

	glUseProgram(0);

	if (GLEW_VERSION_1_3)
	{
		// Disable the multi-texture stuff now that object drawing is done.
		glActiveTexture(GL_TEXTURE1);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE3);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE4);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}

	if (!ModelWindow()->IsRegistered() && (m_bDisplayAO || m_bDisplayColorAO || m_bDisplayNormal))
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glScalef(0.002f, 0.002f, 0.002f);

		glMatrixMode(GL_MODELVIEW);

		static char szFont[1024];
		sprintf(szFont, "%s\\Fonts\\Arial.ttf", getenv("windir"));

		if (m_bDisplayAO || m_bDisplayColorAO)
		{
			glColor4ubv(Color(155, 155, 255, 100));

			CLabel::PaintText("DEMO", 4, "sans-serif", 48, 100, 150);
			CLabel::PaintText("DEMO", 4, "sans-serif", 48, -200, 150);
			CLabel::PaintText("DEMO", 4, "sans-serif", 48, 100, -150);
			CLabel::PaintText("DEMO", 4, "sans-serif", 48, -200, -150);
		}

		glColor4ubv(Color(255, 255, 255, 255));
		tstring sDemoText = "This demo version will generate all map sizes, but will downsample to 128x128 when saving.";
		CLabel::PaintText(sDemoText, sDemoText.length(), "sans-serif", 16, -300.0f, 260.0f);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	}

	if (m_bDisplayUV)
	{
		Vector vecOffset(-0.5f, -0.5f, 0);

		for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		{
			CConversionMesh* pMesh = m_Scene.GetMesh(i);

			if (!pMesh->GetNumUVs())
				continue;

			for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
			{
				size_t k;
				CConversionFace* pFace = pMesh->GetFace(j);

				glBindTexture(GL_TEXTURE_2D, (GLuint)0);
				glColor3f(0.6f, 0.6f, 0.6f);
				glBegin(GL_LINE_STRIP);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(1)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(2)->vu) + vecOffset);
				glEnd();
				for (k = 0; k < pFace->GetNumVertices()-2; k++)
				{
					glBegin(GL_LINES);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vu) + vecOffset);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+2)->vu) + vecOffset);
					glEnd();
				}
				glBegin(GL_LINES);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vu) + vecOffset);
				glEnd();
			}
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
#endif
}

CSMAKRenderer* SMAKRenderer()
{
	return static_cast<CSMAKRenderer*>(SMAKWindow()->GetRenderer());
}

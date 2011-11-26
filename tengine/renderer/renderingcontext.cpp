#include "renderingcontext.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <simplex.h>

#include <models/models.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <models/texturelibrary.h>
#include <renderer/renderer.h>
#include <toys/toy.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CRenderingContext::CRenderingContext(CRenderer* pRenderer)
{
	m_pRenderer = pRenderer;
	m_pShader = NULL;

	m_bMatrixTransformations = false;
	m_bBoundTexture = false;
	m_bFBO = false;
	m_iProgram = 0;
	m_bAttribs = false;

	m_bColorSwap = false;

	m_clrRender = ::Color(255, 255, 255, 255);

	int iWinding;
	glGetIntegerv(GL_FRONT_FACE, &iWinding);
	m_bInitialWinding = (iWinding == GL_CCW);
	m_bReverseWinding = false;

	m_eBlend = BLEND_NONE;
	m_flAlpha = 1;
}

CRenderingContext::~CRenderingContext()
{
	if (m_bMatrixTransformations)
		glPopMatrix();

	if (m_bBoundTexture)
	{
		for (size_t i = 0; i < 8; i++)
		{
			glClientActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, i);
		}
	}

	if (m_bFBO)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_pRenderer->GetSceneBuffer()->m_iFB);
		glViewport(0, 0, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iWidth, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iHeight);
	}

	if (m_iProgram)
		glUseProgram(0);

	if (m_bAttribs)
		glPopAttrib();
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glMultMatrixf(m);
}

void CRenderingContext::Translate(const Vector& vecTranslate)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glTranslatef(vecTranslate.x, vecTranslate.y, vecTranslate.z);
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glRotatef(flAngle, vecAxis.x, vecAxis.y, vecAxis.z);
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glScalef(flX, flY, flZ);
}

void CRenderingContext::ResetTransformations()
{
	if (m_bMatrixTransformations)
	{
		m_bMatrixTransformations = false;
		glPopMatrix();
	}
}

void CRenderingContext::LoadTransform(const Matrix4x4& m)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glLoadMatrixf(m);
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (!m_bAttribs)
		PushAttribs();

	if (eBlend)
	{
		glEnable(GL_BLEND);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	if (!m_bAttribs)
		PushAttribs();

	glDepthMask(bDepthMask);
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CRenderingContext::SetColorSwap(const ::Color& clrSwap)
{
	m_bColorSwap = true;
	m_clrSwap = clrSwap;
}

void CRenderingContext::SetLighting(bool bLighting)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}

void CRenderingContext::SetReverseWinding(bool bReverse)
{
	if (!m_bAttribs)
		PushAttribs();

	m_bReverseWinding = bReverse;
}

void CRenderingContext::RenderModel(size_t iModel, const CBaseEntity* pEntity)
{
	CModel* pModel = CModelLibrary::GetModel(iModel);

	if (!pModel)
		return;

	if (m_pRenderer->IsBatching())
	{
		TAssert(m_eBlend == BLEND_NONE);

		Matrix4x4 mTransformations;
		glGetFloatv(GL_MODELVIEW_MATRIX, mTransformations);

		m_pRenderer->AddToBatch(pModel, pEntity, mTransformations, m_clrRender, m_bColorSwap, m_clrSwap, m_bReverseWinding);
	}
	else
	{
		m_pRenderer->m_pRendering = pEntity;

		glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

		for (size_t m = 0; m < pModel->m_aiVertexBuffers.size(); m++)
		{
			if (!pModel->m_aiVertexBufferSizes[m])
				continue;

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, (GLuint)pModel->m_aiTextures[m]);
			glEnable(GL_TEXTURE_2D);

			RenderModel(pModel, m);
		}

		glUseProgram(0);
		glPopAttrib();

		m_pRenderer->m_pRendering = nullptr;
	}
}

void CRenderingContext::RenderModel(CModel* pModel, size_t iMaterial)
{
	m_pRenderer->SetupShader(this, pModel, iMaterial);

	TAssert(m_pShader);
	if (!m_pShader)
		return;

	if (!pModel || !m_pShader)
		return;

	int iWinding = (m_bInitialWinding?GL_CCW:GL_CW);
	if (m_bReverseWinding)
		iWinding = (m_bInitialWinding?GL_CW:GL_CCW);
	glFrontFace(iWinding);

	glBindBuffer(GL_ARRAY_BUFFER, pModel->m_aiVertexBuffers[iMaterial]);

//	if (m_pShader->m_iNormalAttribute != ~0)
//		glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
//	if (m_pShader->m_iColorAttribute != ~0)
//		glEnableVertexAttribArray(m_pShader->m_iColorAttribute);

	glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);

//	if (m_pShader->m_iNormalAttribute != ~0)
//		glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, sizeof(Vertex_t), BUFFER_OFFSET(((size_t)&v.vecNormal) - ((size_t)&v)));
//	if (m_pShader->m_iColorAttribute != ~0)
//		glVertexAttribPointer(m_pShader->m_iColorAttribute, 3, GL_UNSIGNED_BYTE, true, sizeof(Vertex_t), BUFFER_OFFSET(((size_t)&v.clrColor) - ((size_t)&v)));

	glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, pModel->m_pToy->GetVertexSize(), BUFFER_OFFSET(pModel->m_pToy->GetVertexUV()));
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, pModel->m_pToy->GetVertexSize(), BUFFER_OFFSET(pModel->m_pToy->GetVertexPosition()));

	glDrawArrays(GL_TRIANGLES, 0, pModel->m_aiVertexBufferSizes[iMaterial]);

//	if (m_pShader->m_iNormalAttribute != ~0)
//		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
//	if (m_pShader->m_iColorAttribute != ~0)
//		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CRenderingContext::RenderSphere()
{
	static size_t iSphereCallList = 0;

	if (iSphereCallList == 0)
	{
		GLUquadricObj* pQuadric = gluNewQuadric();
		iSphereCallList = glGenLists(1);
		glNewList((GLuint)iSphereCallList, GL_COMPILE);
		gluSphere(pQuadric, 1, 20, 10);
		glEndList();
		gluDeleteQuadric(pQuadric);
	}

	glCallList(iSphereCallList);
}

void CRenderingContext::RenderBillboard(const tstring& sTexture, float flRadius)
{
	size_t iTexture = CTextureLibrary::FindTextureID(sTexture);

	Vector vecUp, vecRight;
	m_pRenderer->GetCameraVectors(NULL, &vecRight, &vecUp);

	vecUp *= flRadius;
	vecRight *= flRadius;

	BindTexture(iTexture);
	BeginRenderQuads();
		TexCoord(0.0f, 1.0f);
		Vertex(-vecRight + vecUp);
		TexCoord(0.0f, 0.0f);
		Vertex(-vecRight - vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecRight - vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecRight + vecUp);
	EndRender();
}

void CRenderingContext::UseFrameBuffer(const CFrameBuffer* pBuffer)
{
	TAssert(m_pRenderer->ShouldUseFramebuffers());

	m_bFBO = true;
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);
}

void CRenderingContext::UseProgram(const tstring& sProgram)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	m_pShader = CShaderLibrary::GetShader(sProgram);
	if (sProgram.length())
		TAssert(m_pShader);
	if (!m_pShader)
	{
		glUseProgram(0);
		return;
	}

	m_iProgram = m_pShader->m_iProgram;
	glUseProgram((GLuint)m_pShader->m_iProgram);
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const ::Color& clrValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, Vector4D(clrValue));
}

void CRenderingContext::BindTexture(const tstring& sName, int iChannel)
{
	BindTexture(CTextureLibrary::GetTextureGLID(sName), iChannel);
}

void CRenderingContext::BindTexture(size_t iTexture, int iChannel)
{
	if (!m_bAttribs)
		PushAttribs();

	glClientActiveTexture(GL_TEXTURE0+iChannel);
	glActiveTexture(GL_TEXTURE0+iChannel);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glEnable(GL_TEXTURE_2D);

	m_bBoundTexture = true;
}

void CRenderingContext::SetColor(const ::Color& c)
{
	m_clrRender = c;
}

void CRenderingContext::BeginRenderTris()
{
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_TRIANGLES;
}

void CRenderingContext::BeginRenderQuads()
{
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_QUADS;
}

void CRenderingContext::BeginRenderDebugLines()
{
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	glLineWidth( 3.0f );
	m_iDrawMode = GL_LINE_LOOP;
}

void CRenderingContext::TexCoord(float s, float t, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = Vector2D(s, t);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector2D& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const DoubleVector2D& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = Vector2D(v);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const DoubleVector& v, int iChannel)
{
	if (iChannel >= (int)m_avecTexCoord.size())
		m_avecTexCoord.resize(iChannel+1);
	m_avecTexCoord[iChannel] = DoubleVector2D(v);

	m_bTexCoord = true;
}

void CRenderingContext::Normal(const Vector& v)
{
	m_vecNormal = v;
	m_bNormal = true;
}

void CRenderingContext::Color(const ::Color& c)
{
	m_clrColor = c;
	m_bColor = true;
}

void CRenderingContext::Vertex(const Vector& v)
{
	if (m_bTexCoord)
	{
		if (m_aavecTexCoords.size() < m_avecTexCoord.size())
			m_aavecTexCoords.resize(m_avecTexCoord.size());

		for (size_t i = 0; i < m_aavecTexCoords.size(); i++)
			m_aavecTexCoords[i].push_back(m_avecTexCoord[i]);
	}

	if (m_bNormal)
		m_avecNormals.push_back(m_vecNormal);

	if (m_bColor)
		m_aclrColors.push_back(m_clrColor);

	m_avecVertices.push_back(v);
}

void CRenderingContext::RenderCallList(size_t iCallList)
{
	glBegin(GL_TRIANGLES);
	glCallList((GLuint)iCallList);
	glEnd();
}

void CRenderingContext::EndRender()
{
	if (m_bTexCoord && m_pShader->m_iTexCoordAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
		glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, 0, m_aavecTexCoords[0].data());
	}

	if (m_bNormal && m_pShader->m_iNormalAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
		glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, 0, m_avecNormals.data());
	}

	if (m_bColor && m_pShader->m_iColorAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iColorAttribute);
		glVertexAttribPointer(m_pShader->m_iColorAttribute, 3, GL_UNSIGNED_BYTE, true, sizeof(::Color), m_aclrColors.data());
	}

	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, 0, m_avecVertices.data());

	glDrawArrays(m_iDrawMode, 0, m_avecVertices.size());

	glPopClientAttrib();
}

void CRenderingContext::PushAttribs()
{
	m_bAttribs = true;
	// Push all the attribs we'll ever need. I don't want to have to worry about popping them in order.
	glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT|GL_TEXTURE_BIT|GL_POLYGON_BIT);
}


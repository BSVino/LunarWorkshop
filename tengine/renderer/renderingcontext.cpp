#include "renderingcontext.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <simplex.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <models/texturelibrary.h>
#include <renderer/renderer.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CRenderingContext::CRenderingContext(CRenderer* pRenderer)
{
	m_pRenderer = pRenderer;

	m_bMatrixTransformations = false;
	m_bBoundTexture = false;
	m_bFBO = false;
	m_iProgram = 0;
	m_bAttribs = false;

	m_bColorSwap = false;

	m_clrRender = Color(255, 255, 255, 255);

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

void CRenderingContext::SetColorSwap(Color clrSwap)
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

void CRenderingContext::RenderModel(size_t iModel)
{
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);

	if (!pModel)
		return;

	if (m_pRenderer->IsBatching())
	{
		TAssert(m_eBlend == BLEND_NONE);

		Matrix4x4 mTransformations;
		glGetFloatv(GL_MODELVIEW_MATRIX, mTransformations);

		m_pRenderer->AddToBatch(pModel, mTransformations, m_clrRender, m_bColorSwap, m_clrSwap);
	}
	else
	{
		glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

		GLuint iProgram = (GLuint)CShaderLibrary::GetProgram("model");
		glUseProgram(iProgram);

		GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
		glUniform1i(bDiffuse, true);

		GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
		glUniform1i(iDiffuse, 0);

		GLuint vecColor = glGetUniformLocation(iProgram, "vecColor");
		glUniform4fv(vecColor, 1, Vector4D(m_clrRender));

		GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
		glUniform1f(flAlpha, m_flAlpha);

		GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
		glUniform1i(bColorSwapInAlpha, m_bColorSwap);

		if (m_bColorSwap)
		{
			GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
			Vector vecColor((float)m_clrSwap.r()/255, (float)m_clrSwap.g()/255, (float)m_clrSwap.b()/255);
			glUniform3fv(vecColorSwap, 1, vecColor);
		}

		CShader* pShader = CShaderLibrary::GetShader("model");

		TAssert(pShader->m_iPositionAttribute != ~0);
		TAssert(pShader->m_iTexCoordAttribute != ~0);

		for (size_t m = 0; m < pModel->m_aiVertexBuffers.size(); m++)
		{
			if (!pModel->m_aiVertexBufferSizes[m])
				continue;

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, (GLuint)pModel->m_aiTextures[m]);
			glEnable(GL_TEXTURE_2D);

			RenderModel(pModel, m, pShader);
		}

		glUseProgram(0);
		glPopAttrib();
	}
}

void CRenderingContext::RenderModel(CModel* pModel, size_t iMaterial, CShader* pShader)
{
	if (!pShader)
		pShader = m_pShader;

	if (!pModel || !pShader)
		return;

	Vertex_t v;

	glBindBuffer(GL_ARRAY_BUFFER, pModel->m_aiVertexBuffers[iMaterial]);

	if (pShader->m_iNormalAttribute != ~0)
		glEnableVertexAttribArray(pShader->m_iNormalAttribute);

	glEnableVertexAttribArray(pShader->m_iTexCoordAttribute);
	glEnableVertexAttribArray(pShader->m_iPositionAttribute);

	if (pShader->m_iNormalAttribute != ~0)
		glVertexAttribPointer(pShader->m_iNormalAttribute, 3, GL_FLOAT, false, sizeof(Vertex_t), BUFFER_OFFSET(((size_t)&v.vecNormal) - ((size_t)&v)));

	glVertexAttribPointer(pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, sizeof(Vertex_t), BUFFER_OFFSET(((size_t)&v.vecUV) - ((size_t)&v)));
	glVertexAttribPointer(pShader->m_iPositionAttribute, 3, GL_FLOAT, false, sizeof(Vertex_t), BUFFER_OFFSET(((size_t)&v.vecPosition) - ((size_t)&v)));

	glDrawArrays(GL_TRIANGLES, 0, pModel->m_aiVertexBufferSizes[iMaterial]);

	if (pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(pShader->m_iNormalAttribute);

	glDisableVertexAttribArray(pShader->m_iTexCoordAttribute);
	glDisableVertexAttribArray(pShader->m_iPositionAttribute);

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
	TAssert(m_pShader);
	if (!m_pShader)
		return;

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

void CRenderingContext::SetUniform(const char* pszName, const Color& clrValue)
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

void CRenderingContext::SetColor(Color c)
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
	glEnableClientState(GL_NORMAL_ARRAY);
	m_vecNormal = v;
	m_bNormal = true;
}

void CRenderingContext::Vertex(const Vector& v)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	if (m_bTexCoord)
	{
		if (m_aavecTexCoords.size() < m_avecTexCoord.size())
			m_aavecTexCoords.resize(m_avecTexCoord.size());

		for (size_t i = 0; i < m_aavecTexCoords.size(); i++)
			m_aavecTexCoords[i].push_back(m_avecTexCoord[i]);
	}

	if (m_bNormal)
		m_avecNormals.push_back(m_vecNormal);

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
	if (m_bTexCoord)
	{
		glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
		glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, 0, m_aavecTexCoords[0].data());
	}

	if (m_bNormal)
	{
		glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
		glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, 0, m_avecNormals.data());
	}

	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, 0, m_avecVertices.data());

	glDrawArrays(m_iDrawMode, 0, m_avecVertices.size());

	glPopClientAttrib();
}

void CRenderingContext::PushAttribs()
{
	m_bAttribs = true;
	// Push all the attribs we'll ever need. I don't want to have to worry about popping them in order.
	glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT|GL_TEXTURE_BIT);
}


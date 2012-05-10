#include "renderingcontext.h"

#include <GL3/gl3w.h>
#include <GL/glu.h>
#include <FTGL/ftgl.h>

#include <maths.h>
#include <simplex.h>

#include <glgui/label.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>
#include <textures/materiallibrary.h>
#include <renderer/renderer.h>
#include <toys/toy.h>

tvector<CRenderingContext::CRenderContext> CRenderingContext::s_aContexts;

CRenderingContext::CRenderingContext(CRenderer* pRenderer, bool bInherit)
{
	m_pRenderer = pRenderer;

	m_clrRender = ::Color(255, 255, 255, 255);

	s_aContexts.push_back();

	if (bInherit && s_aContexts.size() > 1)
	{
		CRenderContext& oLastContext = s_aContexts[s_aContexts.size()-2];

		GetContext().m_mProjection = oLastContext.m_mProjection;
		GetContext().m_mView = oLastContext.m_mView;
		GetContext().m_mTransformations = oLastContext.m_mTransformations;

		GetContext().m_hMaterial = oLastContext.m_hMaterial;
		GetContext().m_pFrameBuffer = oLastContext.m_pFrameBuffer;
		GetContext().m_sProgram = oLastContext.m_sProgram;

		GetContext().m_eBlend = oLastContext.m_eBlend;
		GetContext().m_flAlpha = oLastContext.m_flAlpha;
		GetContext().m_bDepthMask = oLastContext.m_bDepthMask;
		GetContext().m_bDepthTest = oLastContext.m_bDepthTest;
		GetContext().m_bCull = oLastContext.m_bCull;
		GetContext().m_bWinding = oLastContext.m_bWinding;

		m_pShader = CShaderLibrary::GetShader(GetContext().m_sProgram);

		if (m_pShader)
			m_iProgram = m_pShader->m_iProgram;
		else
			m_iProgram = 0;
	}
	else
	{
		m_pShader = NULL;

		BindTexture(0);
		UseMaterial(CMaterialHandle());
		UseFrameBuffer(NULL);
		UseProgram("");

		SetBlend(BLEND_NONE);
		SetAlpha(1);
		SetDepthMask(true);
		SetDepthTest(true);
		SetBackCulling(true);
		SetWinding(true);
	}
}

CRenderingContext::~CRenderingContext()
{
	TAssert(s_aContexts.size());

	s_aContexts.pop_back();

	if (s_aContexts.size())
	{
		UseMaterial(GetContext().m_hMaterial);
		UseFrameBuffer(GetContext().m_pFrameBuffer);
		UseProgram(GetContext().m_sProgram);

		if (GetContext().m_sProgram.length())
		{
			SetUniform("mProjection", GetContext().m_mProjection);
			SetUniform("mView", GetContext().m_mView);
			SetUniform("mGlobal", GetContext().m_mTransformations);
		}

		SetBlend(GetContext().m_eBlend);
		SetAlpha(GetContext().m_flAlpha);
		SetDepthMask(GetContext().m_bDepthMask);
		SetDepthTest(GetContext().m_bDepthTest);
		SetBackCulling(GetContext().m_bCull);
		SetWinding(GetContext().m_bWinding);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_pRenderer)
			glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight);
		else
			glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight());

		glUseProgram(0);

		glDisablei(GL_BLEND, 0);

		glDepthMask(true);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glFrontFace(GL_CCW);
	}
}

void CRenderingContext::SetProjection(const Matrix4x4& m)
{
	GetContext().m_mProjection = m;

	if (m_pShader)
		SetUniform("mProjection", m);
}

void CRenderingContext::SetView(const Matrix4x4& m)
{
	GetContext().m_mView = m;

	if (m_pShader)
		SetUniform("mView", m);
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	GetContext().m_mTransformations *= m;
}

void CRenderingContext::Translate(const Vector& vecTranslate)
{
	GetContext().m_mTransformations.AddTranslation(vecTranslate);
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	Matrix4x4 mRotation;
	mRotation.SetRotation(flAngle, vecAxis);

	GetContext().m_mTransformations *= mRotation;
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	GetContext().m_mTransformations.AddScale(Vector(flX, flY, flZ));
}

void CRenderingContext::ResetTransformations()
{
	GetContext().m_mTransformations.Identity();
}

void CRenderingContext::LoadTransform(const Matrix4x4& m)
{
	GetContext().m_mTransformations = m;
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (eBlend)
	{
		glEnablei(GL_BLEND, 0);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
		glDisablei(GL_BLEND, 0);

	GetContext().m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	glDepthMask(bDepthMask);
	GetContext().m_bDepthMask = bDepthMask;
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (bDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	GetContext().m_bDepthTest = bDepthTest;
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	GetContext().m_bCull = bCull;
}

void CRenderingContext::SetWinding(bool bWinding)
{
	GetContext().m_bWinding = bWinding;
	glFrontFace(bWinding?GL_CCW:GL_CW);
}

void CRenderingContext::ClearColor(const ::Color& clrClear)
{
	glClearColor((float)(clrClear.r())/255, (float)(clrClear.g())/255, (float)(clrClear.b())/255, (float)(clrClear.a())/255);
	glClear(GL_COLOR_BUFFER_BIT);
}

void CRenderingContext::ClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void CRenderingContext::RenderSphere()
{
	static GLUquadricObj* pQuadric = nullptr;

	if (pQuadric == nullptr)
		pQuadric = gluNewQuadric();

	gluSphere(pQuadric, 1, 20, 10);
}

void CRenderingContext::RenderWireBox(const AABB& aabbBounds)
{
	BeginRenderDebugLines();
		Vertex(aabbBounds.m_vecMaxs);
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(aabbBounds.m_vecMaxs);

		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
	EndRender();

	BeginRenderDebugLines();
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMaxs.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMaxs.z));
	EndRender();

	BeginRenderDebugLines();
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMins.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
	EndRender();

	BeginRenderDebugLines();
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMaxs.y, aabbBounds.m_vecMins.z));
		Vertex(Vector(aabbBounds.m_vecMaxs.x, aabbBounds.m_vecMins.y, aabbBounds.m_vecMins.z));
	EndRender();
}

void CRenderingContext::RenderBillboard(const CMaterialHandle& hMaterial, float flRadius, Vector vecUp, Vector vecRight)
{
	TAssert(hMaterial.IsValid());
	if (!hMaterial.IsValid())
		return;

	vecUp *= flRadius;
	vecRight *= flRadius;

	UseMaterial(hMaterial);
	BeginRenderTriFan();
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
	GetContext().m_pFrameBuffer = pBuffer;

	if (pBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)pBuffer->m_iFB);
		glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_pRenderer)
			glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight);
		else
			glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight());
	}
}

void CRenderingContext::UseProgram(const tstring& sProgram)
{
	GetContext().m_sProgram = sProgram;

	m_pShader = CShaderLibrary::GetShader(sProgram);
	if (sProgram.length())
		TAssert(m_pShader);
	if (!m_pShader)
	{
		m_iProgram = 0;
		glUseProgram(0);
		return;
	}

	m_iProgram = m_pShader->m_iProgram;
	glUseProgram((GLuint)m_pShader->m_iProgram);

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
}

void CRenderingContext::UseMaterial(const CMaterialHandle& hMaterial)
{
	if (!hMaterial.IsValid())
		return;

	GetContext().m_hMaterial = hMaterial;

	UseProgram(hMaterial->m_sShader);

	SetupMaterial();
}

void CRenderingContext::UseMaterial(const tstring& sName)
{
	UseMaterial(CMaterialLibrary::FindMaterial(sName));
}

void CRenderingContext::SetupMaterial()
{
	if (!GetContext().m_hMaterial.IsValid())
		return;

	if (!m_pShader)
		return;

	const tstring& sMaterialBlend = GetContext().m_hMaterial->m_sBlend;
	if (sMaterialBlend == "alpha")
		SetBlend(BLEND_ALPHA);
	else if (sMaterialBlend == "additive")
		SetBlend(BLEND_ADDITIVE);
	else
	{
		TAssert(!sMaterialBlend.length());
		SetBlend(BLEND_NONE);
	}

	for (auto it = m_pShader->m_asUniforms.begin(); it != m_pShader->m_asUniforms.end(); it++)
	{
		auto it2 = m_pShader->m_aDefaults.find(it->first);
		if (it2 == m_pShader->m_aDefaults.end())
		{
			if (it->second == "float")
				SetUniform(it->first.c_str(), 0.0f);
			else if (it->second == "vec2")
				SetUniform(it->first.c_str(), Vector2D());
			else if (it->second == "vec3")
				SetUniform(it->first.c_str(), Vector());
			else if (it->second == "vec4")
				SetUniform(it->first.c_str(), Vector4D());
			else if (it->second == "int")
				SetUniform(it->first.c_str(), 0);
			else if (it->second == "bool")
				SetUniform(it->first.c_str(), false);
			else if (it->second == "mat4")
				SetUniform(it->first.c_str(), Matrix4x4());
			else if (it->second == "sampler2D")
				SetUniform(it->first.c_str(), 0);
			else
				TUnimplemented();
		}
		else
		{
			if (it->second == "float")
				SetUniform(it->first.c_str(), it2->second.m_flValue);
			else if (it->second == "vec2")
				SetUniform(it->first.c_str(), it2->second.m_vec2Value);
			else if (it->second == "vec3")
				SetUniform(it->first.c_str(), it2->second.m_vecValue);
			else if (it->second == "vec4")
				SetUniform(it->first.c_str(), it2->second.m_vec4Value);
			else if (it->second == "int")
				SetUniform(it->first.c_str(), it2->second.m_iValue);
			else if (it->second == "bool")
				SetUniform(it->first.c_str(), it2->second.m_bValue);
			else if (it->second == "mat4")
			{
				TUnimplemented();
			}
			else if (it->second == "sampler2D")
			{
				TUnimplemented();
			}
			else
				TUnimplemented();
		}
	}

	for (size_t i = 0; i < GetContext().m_hMaterial->m_aParameters.size(); i++)
	{
		auto& oParameter = GetContext().m_hMaterial->m_aParameters[i];
		auto& it = m_pShader->m_aParameters.find(oParameter.m_sName);

		TAssert(it != m_pShader->m_aParameters.end());
		if (it == m_pShader->m_aParameters.end())
			continue;

		for (size_t j = 0; j < it->second.m_aActions.size(); j++)
		{
			auto& oAction = it->second.m_aActions[j];
			tstring& sName = oAction.m_sName;
			tstring& sValue = oAction.m_sValue;
			tstring& sType = m_pShader->m_asUniforms[sName];
			if (sValue == "[value]")
			{
				if (sType == "float")
					SetUniform(sName.c_str(), oParameter.m_flValue);
				else if (sType == "vec2")
					SetUniform(sName.c_str(), oParameter.m_vec2Value);
				else if (sType == "vec3")
					SetUniform(sName.c_str(), oParameter.m_vecValue);
				else if (sType == "vec4")
					SetUniform(sName.c_str(), oParameter.m_vec4Value);
				else if (sType == "int")
					SetUniform(sName.c_str(), oParameter.m_iValue);
				else if (sType == "bool")
					SetUniform(sName.c_str(), oParameter.m_bValue);
				else if (sType == "mat4")
				{
					TUnimplemented();
				}
				else if (sType == "sampler2D")
				{
					// No op, handled below.
				}
				else
					TUnimplemented();
			}
			else
			{
				if (sType == "float")
					SetUniform(sName.c_str(), oAction.m_flValue);
				else if (sType == "vec2")
					SetUniform(sName.c_str(), oAction.m_vec2Value);
				else if (sType == "vec3")
					SetUniform(sName.c_str(), oAction.m_vecValue);
				else if (sType == "vec4")
					SetUniform(sName.c_str(), oAction.m_vec4Value);
				else if (sType == "int")
					SetUniform(sName.c_str(), oAction.m_iValue);
				else if (sType == "bool")
					SetUniform(sName.c_str(), oAction.m_bValue);
				else if (sType == "mat4")
				{
					TUnimplemented();
				}
				else if (sType == "sampler2D")
				{
					TUnimplemented();
					SetUniform(sName.c_str(), 0);
				}
				else
					TUnimplemented();
			}
		}	
	}

	for (size_t i = 0; i < m_pShader->m_asTextures.size(); i++)
	{
		if (!GetContext().m_hMaterial->m_ahTextures[i].IsValid())
			continue;

		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, (GLuint)GetContext().m_hMaterial->m_ahTextures[i]->m_iGLID);
		SetUniform(m_pShader->m_asTextures[i].c_str(), (int)i);
	}
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector4D& vecValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const ::Color& clrValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, Vector4D(clrValue));
}

void CRenderingContext::SetUniform(const char* pszName, const Matrix4x4& mValue)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniformMatrix4fv(iUniform, 1, false, mValue);
}

void CRenderingContext::SetUniform(const char* pszName, size_t iSize, const float* aflValues)
{
	TAssert(m_pShader);
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1fv(iUniform, iSize, aflValues);
}

void CRenderingContext::BindTexture(size_t iTexture, int iChannel)
{
	// Not tested since the move to a stack
	TAssert(iChannel == 0);

	glActiveTexture(GL_TEXTURE0+iChannel);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
}

void CRenderingContext::BindBufferTexture(const CFrameBuffer& oBuffer, int iChannel)
{
	glActiveTexture(GL_TEXTURE0+iChannel);

	if (oBuffer.m_bMultiSample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)oBuffer.m_iMap);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);

	// Not ported to texture handle code.
//	GetContext().m_hTexture = oBuffer.m_iMap;
}

void CRenderingContext::SetColor(const ::Color& c)
{
	m_clrRender = c;
}

void CRenderingContext::BeginRenderTris()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_TRIANGLES;
}

void CRenderingContext::BeginRenderTriFan()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	m_iDrawMode = GL_TRIANGLE_FAN;
}

void CRenderingContext::BeginRenderQuads()
{
	m_avecTexCoord.clear();
	m_aavecTexCoords.clear();
	m_avecNormals.clear();
	m_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;
	m_bColor = false;

	TUnimplemented();
	//m_iDrawMode = GL_QUADS;
}

void CRenderingContext::BeginRenderDebugLines()
{
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

void CRenderingContext::EndRender()
{
	TAssert(m_pShader);
	if (!m_pShader)
	{
		UseProgram("model");
		if (!m_pShader)
			return;
	}

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

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

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);
}

void CRenderingContext::BeginRenderVertexArray(size_t iBuffer)
{
	if (iBuffer)
		glBindBuffer(GL_ARRAY_BUFFER, iBuffer);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void CRenderingContext::SetPositionBuffer(float* pflBuffer, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetPositionBuffer(size_t iOffset, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetTexCoordBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iTexCoordAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetTexCoordBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iTexCoordAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	glVertexAttribPointer(m_pShader->m_iTexCoordAttribute, 2, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride)
{
	int iAttribute = glGetAttribLocation(m_iProgram, pszName);

	TAssert(iAttribute != ~0);
	if (iAttribute == ~0)
		return;

	glEnableVertexAttribArray(iAttribute);
	glVertexAttribIPointer(iAttribute, iSize, GL_INT, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::EndRenderVertexArray(size_t iVertices)
{
	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

	glDrawArrays(GL_TRIANGLES, 0, iVertices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CRenderingContext::EndRenderVertexArrayTriangles(size_t iTriangles, int* piIndices)
{
	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);
	SetUniform("mGlobal", GetContext().m_mTransformations);

	glDrawElements(GL_TRIANGLES, iTriangles*3, GL_UNSIGNED_INT, piIndices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	if (m_pShader->m_iTexCoordAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTexCoordAttribute);
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CRenderingContext::RenderText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize)
{
	TAssert(m_pShader);
	if (!m_pShader)
		return;

	TAssert(m_pShader->m_iPositionAttribute >= 0);
	TAssert(m_pShader->m_iTexCoordAttribute >= 0);

	if (!glgui::CLabel::GetFont(sFontName, iFontFaceSize))
		glgui::CLabel::AddFontSize(sFontName, iFontFaceSize);

	SetUniform("mProjection", GetContext().m_mProjection);
	SetUniform("mView", GetContext().m_mView);

	// Take the position out and let FTGL do it. It looks sharper that way.
	Matrix4x4 mTransformations = GetContext().m_mTransformations;
	Vector vecPosition = mTransformations.GetTranslation();
	mTransformations.SetTranslation(Vector());
	SetUniform("mGlobal", mTransformations);

	ftglSetAttributeLocations(m_pShader->m_iPositionAttribute, m_pShader->m_iTexCoordAttribute);
	glgui::CLabel::GetFont(sFontName, iFontFaceSize)->Render(sText.c_str(), iLength, FTPoint(vecPosition.x, vecPosition.y, vecPosition.z));
}

CRenderingContext::CRenderContext& CRenderingContext::GetContext()
{
	return s_aContexts.back();
}

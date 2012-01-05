#ifndef TINKER_RENDERINGCONTEXT_H
#define TINKER_RENDERINGCONTEXT_H

#include <tstring.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

#include "render_common.h"

class CRenderingContext
{
public:
							CRenderingContext(class CRenderer* pRenderer = nullptr);
	virtual					~CRenderingContext();

public:
	void					SetProjection(const Matrix4x4& m);
	void					SetView(const Matrix4x4& m);

	Matrix4x4				GetProjection() { return m_mProjection; }
	Matrix4x4				GetView() { return m_mView; }

	void					Transform(const Matrix4x4& m);
	void					Translate(const Vector& vecTranslate);
	void					Rotate(float flAngle, Vector vecAxis);
	void					Scale(float flX, float flY, float flZ);
	void					ResetTransformations();
	void					LoadTransform(const Matrix4x4& m);

	void					SetBlend(blendtype_t eBlend);
	void					SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };
	void					SetDepthMask(bool bDepthMask);
	void					SetDepthTest(bool bDepthTest);
	void					SetBackCulling(bool bCull);
	void					SetColorSwap(const ::Color& clrSwap);
	void					SetReverseWinding(bool bReverse);

	float					GetAlpha() { return m_flAlpha; };
	blendtype_t				GetBlend() { return m_eBlend; };
	::Color					GetColor() { return m_clrRender; };
	bool					IsColorSwapActive() { return m_bColorSwap; }
	::Color					GetColorSwap() { return m_clrSwap; }

	void					RenderSphere();

	void					RenderBillboard(const tstring& sTexture, float flRadius, Vector vecUp, Vector vecRight);

	void					UseFrameBuffer(const class CFrameBuffer* pBuffer);
	void					UseProgram(const tstring& sProgram);
	size_t					GetActiveProgram() { return m_iProgram; }
	class CShader*			GetActiveShader() { return m_pShader; }
	void					SetUniform(const char* pszName, int iValue);
	void					SetUniform(const char* pszName, float flValue);
	void					SetUniform(const char* pszName, const Vector& vecValue);
	void					SetUniform(const char* pszName, const Vector4D& vecValue);
	void					SetUniform(const char* pszName, const ::Color& vecValue);
	void					SetUniform(const char* pszName, const Matrix4x4& mValue);
	void					SetUniform(const char* pszName, size_t iSize, const float* aflValues);
	void					BindTexture(const tstring& sName, int iChannel = 0);
	void					BindTexture(size_t iTexture, int iChannel = 0);
	void					BindBufferTexture(const CFrameBuffer& oBuffer, int iChannel = 0);
	void					SetColor(const ::Color& c);	// Set the mesh's uniform color. Do this before BeginRender*

	// Immediate mode emulation
	void					BeginRenderTris();
	void					BeginRenderQuads();
	void					BeginRenderDebugLines();
	void					TexCoord(float s, float t, int iChannel = 0);
	void					TexCoord(const Vector2D& v, int iChannel = 0);
	void					TexCoord(const DoubleVector2D& v, int iChannel = 0);
	void					TexCoord(const Vector& v, int iChannel = 0);
	void					TexCoord(const DoubleVector& v, int iChannel = 0);
	void					Normal(const Vector& v);
	void					Color(const ::Color& c);	// Per-attribute color
	void					Vertex(const Vector& v);
	void					EndRender();

	void					BeginRenderVertexArray(size_t iBuffer=0);
	void					SetPositionBuffer(float* pflBuffer, size_t iStride=0);
	void					SetPositionBuffer(size_t iOffset, size_t iStride);
	void					SetTexCoordBuffer(float* pflBuffer, size_t iStride=0);
	void					SetTexCoordBuffer(size_t iOffset, size_t iStride);
	void					SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride);
	void					EndRenderVertexArray(size_t iVertices);

public:
	class CRenderer*		m_pRenderer;

	Matrix4x4				m_mProjection;
	Matrix4x4				m_mView;
	Matrix4x4				m_mTransformations;

	bool					m_bBoundTexture;
	bool					m_bFBO;
	size_t					m_iProgram;
	class CShader*			m_pShader;
	bool					m_bAttribs;

	bool					m_bColorSwap;
	::Color					m_clrSwap;

	::Color					m_clrRender;

	bool					m_bInitialWinding;
	bool					m_bReverseWinding;

	blendtype_t				m_eBlend;
	float					m_flAlpha;

	int						m_iDrawMode;
	bool					m_bTexCoord;
	bool					m_bNormal;
	bool					m_bColor;
	eastl::vector<Vector2D>	m_avecTexCoord;
	eastl::vector<eastl::vector<Vector2D> >	m_aavecTexCoords;
	Vector					m_vecNormal;
	eastl::vector<Vector>	m_avecNormals;
	::Color					m_clrColor;
	eastl::vector<::Color>	m_aclrColors;
	eastl::vector<Vector>	m_avecVertices;
};

#endif

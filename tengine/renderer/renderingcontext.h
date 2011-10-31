#ifndef TINKER_RENDERINGCONTEXT_H
#define TINKER_RENDERINGCONTEXT_H

#include <tstring.h>
#include <tengine_config.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

#include "render_common.h"

class CRenderingContext
{
public:
							CRenderingContext(class CRenderer* pRenderer);
							~CRenderingContext();

public:
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
	void					SetLighting(bool bLighting);

	float					GetAlpha() { return m_flAlpha; };
	blendtype_t				GetBlend() { return m_eBlend; };
	::Color					GetColor() { return m_clrRender; };
	bool					IsColorSwapActive() { return m_bColorSwap; }
	::Color					GetColorSwap() { return m_clrSwap; }

	void					RenderModel(size_t iModel);
	void					RenderModel(class CModel* pModel, size_t iMaterial);

	void					RenderSphere();

	void					RenderBillboard(const tstring& sTexture, float flRadius);

	void					UseFrameBuffer(const class CFrameBuffer* pBuffer);
	void					UseProgram(const tstring& sProgram);
	void					SetUniform(const char* pszName, int iValue);
	void					SetUniform(const char* pszName, float flValue);
	void					SetUniform(const char* pszName, const Vector& vecValue);
	void					SetUniform(const char* pszName, const ::Color& vecValue);
	void					BindTexture(const tstring& sName, int iChannel = 0);
	void					BindTexture(size_t iTexture, int iChannel = 0);
	void					SetColor(const ::Color& c);	// Set the mesh's uniform color. Do this before BeginRender*
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
	void					RenderCallList(size_t iCallList);
	void					EndRender();

protected:
	void					PushAttribs();

public:
	CRenderer*				m_pRenderer;

	bool					m_bMatrixTransformations;
	bool					m_bBoundTexture;
	bool					m_bFBO;
	size_t					m_iProgram;
	class CShader*			m_pShader;
	bool					m_bAttribs;

	bool					m_bColorSwap;
	::Color					m_clrSwap;

	::Color					m_clrRender;

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

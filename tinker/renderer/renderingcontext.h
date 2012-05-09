#ifndef TINKER_RENDERINGCONTEXT_H
#define TINKER_RENDERINGCONTEXT_H

#include <tstring.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>
#include <geometry.h>

#include <textures/materialhandle.h>

#include "render_common.h"

class CRenderingContext
{
protected:
	class CRenderContext
	{
	public:
		Matrix4x4			m_mProjection;
		Matrix4x4			m_mView;
		Matrix4x4			m_mTransformations;

		CMaterialHandle		m_hMaterial;
		const class CFrameBuffer*	m_pFrameBuffer;
		tstring				m_sProgram;

		blendtype_t			m_eBlend;
		float				m_flAlpha;
		bool				m_bDepthMask;
		bool				m_bDepthTest;
		bool				m_bCull;
		bool				m_bWinding;
	};

public:
							CRenderingContext(class CRenderer* pRenderer = nullptr, bool bInherit = false);	// Make bInherit true if you want to preserve and not clobber GL settings set previously
	virtual					~CRenderingContext();

public:
	void					SetProjection(const Matrix4x4& m);
	void					SetView(const Matrix4x4& m);

	Matrix4x4				GetProjection() { return GetContext().m_mProjection; }
	Matrix4x4				GetView() { return GetContext().m_mView; }

	void					Transform(const Matrix4x4& m);
	void					Translate(const Vector& vecTranslate);
	void					Rotate(float flAngle, Vector vecAxis);
	void					Scale(float flX, float flY, float flZ);
	void					ResetTransformations();
	void					LoadTransform(const Matrix4x4& m);

	void					SetBlend(blendtype_t eBlend);
	void					SetAlpha(float flAlpha) { GetContext().m_flAlpha = flAlpha; };
	void					SetDepthMask(bool bDepthMask);
	void					SetDepthTest(bool bDepthTest);
	void					SetBackCulling(bool bCull);
	void					SetWinding(bool bWinding);	// True is default

	float					GetAlpha() { return GetContext().m_flAlpha; };
	blendtype_t				GetBlend() { return GetContext().m_eBlend; };
	::Color					GetColor() { return m_clrRender; };
	bool					GetWinding() { return GetContext().m_bWinding; };

	void					ClearColor(const ::Color& clrClear = ::Color(0.0f, 0.0f, 0.0f, 1.0f));
	void					ClearDepth();

	void					RenderSphere();
	void					RenderWireBox(const AABB& aabbBounds);

	void					RenderBillboard(const CMaterialHandle& hMaterial, float flRadius, Vector vecUp, Vector vecRight);

	void					UseFrameBuffer(const class CFrameBuffer* pBuffer);
	const class CFrameBuffer* GetActiveFrameBuffer() { return GetContext().m_pFrameBuffer; }
	void					UseProgram(const tstring& sProgram);
	void					UseMaterial(const CMaterialHandle& hMaterial);
	void					UseMaterial(const tstring& sName);
	void					SetupMaterial();
	size_t					GetActiveProgram() { return m_iProgram; }
	class CShader*			GetActiveShader() { return m_pShader; }
	void					SetUniform(const char* pszName, int iValue);
	void					SetUniform(const char* pszName, float flValue);
	void					SetUniform(const char* pszName, const Vector& vecValue);
	void					SetUniform(const char* pszName, const Vector4D& vecValue);
	void					SetUniform(const char* pszName, const ::Color& vecValue);
	void					SetUniform(const char* pszName, const Matrix4x4& mValue);
	void					SetUniform(const char* pszName, size_t iSize, const float* aflValues);
	void					BindTexture(size_t iTexture, int iChannel = 0);
	void					BindBufferTexture(const CFrameBuffer& oBuffer, int iChannel = 0);
	void					SetColor(const ::Color& c);	// Set the mesh's uniform color. Do this before BeginRender*

	// Immediate mode emulation
	void					BeginRenderTris();
	void					BeginRenderTriFan();
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
	void					EndRenderVertexArrayTriangles(size_t iTriangles, int* piIndices);

	void					RenderText(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize);

protected:
	inline CRenderContext&	GetContext();

public:
	class CRenderer*		m_pRenderer;
	class CShader*			m_pShader;

	size_t					m_iProgram;

	::Color					m_clrRender;

	int						m_iDrawMode;
	bool					m_bTexCoord;
	bool					m_bNormal;
	bool					m_bColor;
	tvector<Vector2D>		m_avecTexCoord;
	tvector<tvector<Vector2D> >	m_aavecTexCoords;
	Vector					m_vecNormal;
	tvector<Vector>	m_avecNormals;
	::Color					m_clrColor;
	tvector<::Color>		m_aclrColors;
	tvector<Vector>			m_avecVertices;

	static tvector<CRenderContext>	s_aContexts;
};

#endif

#include "shaders.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <assert.h>

CShaderLibrary* CShaderLibrary::s_pShaderLibrary = NULL;
static CShaderLibrary g_ShaderLibrary = CShaderLibrary();

CShaderLibrary::CShaderLibrary()
{
	s_pShaderLibrary = this;

	m_bCompiled = false;

	m_iTerrain = AddShader(GetVSTerrainShader(), GetFSTerrainShader());
	m_iModel = AddShader(GetVSModelShader(), GetFSModelShader());

	m_iBlur = AddShader(GetVSPassShader(), GetFSBlurShader());
	m_iBrightPass = AddShader(GetVSPassShader(), GetFSBrightPassShader());
}

CShaderLibrary::~CShaderLibrary()
{
	for (size_t i = 0; i < m_aShaders.size(); i++)
	{
		CShader* pShader = &m_aShaders[i];
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
		glDeleteProgram((GLuint)pShader->m_iProgram);
		glDeleteShader((GLuint)pShader->m_iVShader);
		glDeleteShader((GLuint)pShader->m_iFShader);
	}

	s_pShaderLibrary = NULL;
}

size_t CShaderLibrary::AddShader(const char* pszVS, const char* pszFS)
{
	if (m_bCompiled)
		return ~0;

	m_aShaders.push_back(CShader(pszVS, pszFS));
	return m_aShaders.size()-1;
}

void CShaderLibrary::CompileShaders()
{
	if (Get()->m_bCompiled)
		return;

	for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		Get()->CompileShader(i);

	Get()->m_bCompiled = true;
}

void CShaderLibrary::CompileShader(size_t iShader)
{
	CShader* pShader = &m_aShaders[iShader];

	pShader->m_iVShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszStr = pShader->m_sVS.c_str();
	glShaderSource((GLuint)pShader->m_iVShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)pShader->m_iVShader);

#ifdef _DEBUG
	int iLogLength = 0;
	char szLog[1024];
	glGetShaderInfoLog((GLuint)pShader->m_iVShader, 1024, &iLogLength, szLog);
#endif

	pShader->m_iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszStr = pShader->m_sFS.c_str();
	glShaderSource((GLuint)pShader->m_iFShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)pShader->m_iFShader);

#ifdef _DEBUG
	glGetShaderInfoLog((GLuint)pShader->m_iFShader, 1024, &iLogLength, szLog);
#endif

	pShader->m_iProgram = glCreateProgram();
	glAttachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
	glAttachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
	glLinkProgram((GLuint)pShader->m_iProgram);

#ifdef _DEBUG
	glGetProgramInfoLog((GLuint)pShader->m_iProgram, 1024, &iLogLength, szLog);
	assert(!strlen(szLog));
#endif
}

CShader* CShaderLibrary::GetShader(size_t i)
{
	if (i >= m_aShaders.size())
		return NULL;

	return &m_aShaders[i];
}

CShader::CShader(const char* pszVS, const char* pszFS)
{
	m_sVS = pszVS;
	m_sFS = pszFS;
	m_iVShader = 0;
	m_iFShader = 0;
	m_iProgram = 0;
}

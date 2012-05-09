#include "shaders.h"

#include <GL3/gl3w.h>
#include <time.h>

#include <common.h>
#include <tinker_platform.h>
#include <worklistener.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>

CShaderLibrary* CShaderLibrary::s_pShaderLibrary = NULL;
static CShaderLibrary g_ShaderLibrary = CShaderLibrary();

CShaderLibrary::CShaderLibrary()
{
	s_pShaderLibrary = this;

	m_bCompiled = false;
	m_iSamples = -1;

	FILE* f = tfopen("shaders/functions.si", "r");

	TAssert(f);
	if (f)
	{
		tstring sLine;
		while (fgetts(sLine, f))
			m_sFunctions += sLine;

		fclose(f);
	}

	f = tfopen("shaders/header.si", "r");

	TAssert(f);
	if (f)
	{
		tstring sLine;
		while (fgetts(sLine, f))
			m_sHeader += sLine;

		fclose(f);
	}
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

void CShaderLibrary::AddShader(const tstring& sFile)
{
	TAssert(!Get()->m_bCompiled);
	if (Get()->m_bCompiled)
		return;

	std::basic_ifstream<tchar> f(sFile.c_str());

	if (!f.is_open())
	{
		TError("Couldn't open shader file: " + sFile + "\n");
		return;
	}

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pName = pData->FindChild("Name");
	CData* pVertex = pData->FindChild("Vertex");
	CData* pFragment = pData->FindChild("Fragment");

	TAssert(pName);
	if (!pName)
	{
		TError("Malformed shader file. " + sFile + " has no Name entry.\n");
		return;
	}

	TAssert(pVertex);
	if (!pVertex)
	{
		TError("Malformed shader file. " + sFile + " has no Vertex entry.\n");
		return;
	}

	TAssert(pFragment);
	if (!pFragment)
	{
		TError("Malformed shader file. " + sFile + " has no Fragment entry.\n");
		return;
	}

	Get()->m_aShaders.push_back(CShader(pName->GetValueTString(), pVertex->GetValueTString(), pFragment->GetValueTString()));
	Get()->m_aShaderNames[pName->GetValueTString()] = Get()->m_aShaders.size()-1;

	auto& oShader = Get()->m_aShaders.back();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);
		if (pChild->GetKey() == "Parameter")
		{
			auto& oParameter = oShader.m_aParameters.insert(pChild->GetValueTString()).first->second;
			oParameter.m_sName = pChild->GetValueTString();

			for (size_t j = 0; j < pChild->GetNumChildren(); j++)
			{
				CData* pUniform = pChild->GetChild(j);
				if (pUniform->GetKey() == "Uniform")
				{
					auto& oUniform = oParameter.m_aActions.push_back();
					oUniform.m_sName = pUniform->GetValueTString();
					oUniform.m_bTexture = false;
					CData* pValue = pUniform->FindChild("Value");
					CData* pTexture = pUniform->FindChild("Texture");
					TAssert(!(pValue && pTexture));
					TAssert(pValue || pTexture);

					if (pValue)
						oUniform.m_sValue = pValue->GetValueTString();
					else if (pTexture)
					{
						oUniform.m_sValue = pTexture->GetValueTString();
						oShader.m_asTextures.push_back(pUniform->GetValueTString());
						oUniform.m_bTexture = true;
					}
				}
				else if (pUniform->GetKey() == "Blend")
				{
					tstring& sBlend = oParameter.m_sBlend;
					sBlend = pUniform->GetValueTString();
				}
			}
		}
		else if (pChild->GetKey() == "Defaults")
		{
			for (size_t j = 0; j < pChild->GetNumChildren(); j++)
			{
				CData* pUniform = pChild->GetChild(j);
				auto& oDefault = oShader.m_aDefaults.insert(pUniform->GetKey());
				oDefault.first->second.m_sName = pUniform->GetKey();
				oDefault.first->second.m_sValue = pUniform->GetValueTString();
			}
		}
	}
}

void CShaderLibrary::CompileShaders(int iSamples)
{
	if (iSamples != -1)
		Get()->m_iSamples = iSamples;

	TAssert(Get()->m_iSamples != -1);

	Get()->ClearLog();

//	if (GameServer()->GetWorkListener())
//		GameServer()->GetWorkListener()->SetAction("Compiling shaders", Get()->m_aShaders.size());

	if (Get()->m_bCompiled)
	{
		// If this is a recompile just blow through them.
		for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
			Get()->m_aShaders[i].Compile();
	}
	else
	{
		bool bShadersCompiled = true;
		for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		{
			bShadersCompiled &= Get()->m_aShaders[i].Compile();

			if (!bShadersCompiled)
				break;

	//		if (GameServer()->GetWorkListener())
	//			GameServer()->GetWorkListener()->WorkProgress(i);
		}

		if (bShadersCompiled)
			Get()->m_bCompiled = true;
	}
}

void CShaderLibrary::DestroyShaders()
{
	for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		Get()->m_aShaders[i].Destroy();

	Get()->m_bCompiled = false;
}

void CShaderLibrary::ClearLog()
{
	m_bLogNeedsClearing = true;
}

void CShaderLibrary::WriteLog(const char* pszLog, const char* pszShaderText)
{
	if (!pszLog || strlen(pszLog) == 0)
		return;

	tstring sFile = GetAppDataDirectory(Application()->AppDirectory(), "shaders.txt");

	if (m_bLogNeedsClearing)
	{
		// Only clear it if we're actually going to write to it so we don't create the file.
		FILE* fp = tfopen(sFile, "w");
		fclose(fp);
		m_bLogNeedsClearing = false;
	}

	char szText[100];
	strncpy(szText, pszShaderText, 99);
	szText[99] = '\0';

	FILE* fp = tfopen(sFile, "a");
	fprintf(fp, "Shader compile output %d\n", (int)time(NULL));
	fprintf(fp, "%s\n\n", pszLog);
	fprintf(fp, "%s...\n\n", szText);
	fclose(fp);
}

CShader* CShaderLibrary::GetShader(const tstring& sName)
{
	eastl::map<tstring, size_t>::const_iterator i = Get()->m_aShaderNames.find(sName);
	if (i == Get()->m_aShaderNames.end())
		return NULL;

	return &Get()->m_aShaders[i->second];
}

CShader* CShaderLibrary::GetShader(size_t i)
{
	if (i >= Get()->m_aShaders.size())
		return NULL;

	return &Get()->m_aShaders[i];
}

size_t CShaderLibrary::GetProgram(const tstring& sName)
{
	TAssert(Get());
	if (!Get())
		return 0;

	TAssert(Get()->GetShader(sName));
	if (!Get()->GetShader(sName))
		return 0;

	return Get()->GetShader(sName)->m_iProgram;
}

CShader::CShader(const tstring& sName, const tstring& sVertexFile, const tstring& sFragmentFile)
{
	m_sName = sName;
	m_sVertexFile = sVertexFile;
	m_sFragmentFile = sFragmentFile;
	m_iVShader = 0;
	m_iFShader = 0;
	m_iProgram = 0;
}

bool CShader::Compile()
{
	tstring sShaderHeader = CShaderLibrary::GetShaderHeader();

	if (CShaderLibrary::Get()->m_iSamples)
		sShaderHeader += "#define USE_MULTISAMPLE_TEXTURES 1\n";

	sShaderHeader += CShaderLibrary::GetShaderFunctions();

	FILE* f = tfopen("shaders/" + m_sVertexFile + ".vs", "r");

	TAssert(f);
	if (!f)
		return false;

	tstring sVertexShader = sShaderHeader;
	sVertexShader += "uniform mat4x4 mProjection;\n";
	sVertexShader += "uniform mat4x4 mView;\n";
	sVertexShader += "uniform mat4x4 mGlobal;\n";

	tstring sLine;
	while (fgetts(sLine, f))
		sVertexShader += sLine;

	fclose(f);

	f = tfopen("shaders/" + m_sFragmentFile + ".fs", "r");

	TAssert(f);
	if (!f)
		return false;

	tstring sFragmentShader = sShaderHeader;
	sFragmentShader += "out vec4 vecOutputColor;\n";

	while (fgetts(sLine, f))
		sFragmentShader += sLine;

	fclose(f);

	size_t iVShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszStr = sVertexShader.c_str();
	glShaderSource((GLuint)iVShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)iVShader);

	int iVertexCompiled;
	glGetShaderiv((GLuint)iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	if (iVertexCompiled != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iVShader, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog(szLog, pszStr);
	}

	size_t iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszStr = sFragmentShader.c_str();
	glShaderSource((GLuint)iFShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)iFShader);

	int iFragmentCompiled;
	glGetShaderiv((GLuint)iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	if (iFragmentCompiled != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iFShader, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog(szLog, pszStr);
	}

	size_t iProgram = glCreateProgram();

	glBindAttribLocation(iProgram, 0, "vecPosition");		// Force position at location 0. ATI cards won't work without this.

	glAttachShader((GLuint)iProgram, (GLuint)iVShader);
	glAttachShader((GLuint)iProgram, (GLuint)iFShader);
	glLinkProgram((GLuint)iProgram);

	int iProgramLinked;
	glGetProgramiv((GLuint)iProgram, GL_LINK_STATUS, &iProgramLinked);

	if (iProgramLinked != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetProgramInfoLog((GLuint)iProgram, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog(szLog, "link");
	}

	if (iVertexCompiled != GL_TRUE || iFragmentCompiled != GL_TRUE || iProgramLinked != GL_TRUE)
	{
		TError("Shader compilation failed for shader " + m_sName + ". Check shaders.txt\n");

		glDetachShader((GLuint)iProgram, (GLuint)iVShader);
		glDetachShader((GLuint)iProgram, (GLuint)iFShader);
		glDeleteShader((GLuint)iVShader);
		glDeleteShader((GLuint)iFShader);
		glDeleteProgram((GLuint)iProgram);

		return false;
	}

	Destroy();

	m_iProgram = iProgram;
	m_iVShader = iVShader;
	m_iFShader = iFShader;

	m_iPositionAttribute = glGetAttribLocation(m_iProgram, "vecPosition");
	m_iNormalAttribute = glGetAttribLocation(m_iProgram, "vecNormal");
	m_iTexCoordAttribute = glGetAttribLocation(m_iProgram, "vecTexCoord0");
	m_iColorAttribute = glGetAttribLocation(m_iProgram, "vecVertexColor");

	glBindFragDataLocation(m_iProgram, 0, "vecOutputColor");

	TAssert(m_iPositionAttribute != ~0);

	int iNumUniforms;
	glGetProgramiv(m_iProgram, GL_ACTIVE_UNIFORMS, &iNumUniforms);

	char szUniformName[256];
	GLsizei iLength;
	GLint iSize;
	GLenum iType;
	for (int i = 0; i < iNumUniforms; i++)
	{
		glGetActiveUniform(m_iProgram, i, sizeof(szUniformName), &iLength, &iSize, &iType, szUniformName);

		tstring sUniformName = szUniformName;
		if (sUniformName == "mProjection")
			continue;
		if (sUniformName == "mView")
			continue;
		if (sUniformName == "mGlobal")
			continue;

		tstring& sType = m_asUniforms.insert(sUniformName).first->second;
		switch (iType)
		{
		case GL_FLOAT: sType = "float"; break;
		case GL_FLOAT_VEC2: sType = "vec2"; break;
		case GL_FLOAT_VEC3: sType = "vec3"; break;
		case GL_FLOAT_VEC4: sType = "vec4"; break;
		case GL_INT: sType = "int"; break;
		case GL_BOOL: sType = "bool"; break;
		case GL_FLOAT_MAT4: sType = "mat4"; break;
		case GL_SAMPLER_2D: sType = "sampler2D"; break;
		default: TUnimplemented();
		}
	}

	for (auto it = m_aParameters.begin(); it != m_aParameters.end(); it++)
	{
		for (size_t j = 0; j < it->second.m_aActions.size(); j++)
		{
			auto it2 = m_asUniforms.find(it->second.m_aActions[j].m_sName);
			TAssert(it2 != m_asUniforms.end());
			if (it2 == m_asUniforms.end())
			{
				TError("Shader '" + m_sName + "' specifies a uniform '" + it->second.m_aActions[j].m_sName + "' that is not in the linked program.\n");
				continue;
			}

			tstring& sType = it2->second;

			// This is almost cheating
			CData d;
			d.SetValue(it->second.m_aActions[j].m_sValue);

			if (sType == "float")
				it->second.m_aActions[j].m_flValue = d.GetValueFloat();
			else if (sType == "vec2")
				it->second.m_aActions[j].m_vec2Value = d.GetValueVector2D();
			else if (sType == "vec3")
				it->second.m_aActions[j].m_vecValue = d.GetValueVector();
			else if (sType == "vec4")
				it->second.m_aActions[j].m_vec4Value = d.GetValueVector4D();
			else if (sType == "int")
				it->second.m_aActions[j].m_iValue = d.GetValueInt();
			else if (sType == "bool")
				it->second.m_aActions[j].m_bValue = d.GetValueBool();
			else if (sType == "mat4")
			{
				TUnimplemented();
			}
			else if (sType == "sampler2D")
			{
				// No op.
			}
			else
				TUnimplemented();
		}
	}

	for (auto it = m_aDefaults.begin(); it != m_aDefaults.end(); it++)
	{
		auto it2 = m_asUniforms.find(it->first);
		TAssert(it2 != m_asUniforms.end());
		if (it2 == m_asUniforms.end())
		{
			TError("Shader '" + m_sName + "' specifies a default for uniform '" + it->second.m_sName + "' that is not in the linked program.\n");
			continue;
		}

		tstring& sType = it2->second;

		// Again with the cheating.
		CData d;
		d.SetValue(it->second.m_sValue);

		if (sType == "float")
			it->second.m_flValue = d.GetValueFloat();
		else if (sType == "vec2")
			it->second.m_vec2Value = d.GetValueVector2D();
		else if (sType == "vec3")
			it->second.m_vecValue = d.GetValueVector();
		else if (sType == "vec4")
			it->second.m_vec4Value = d.GetValueVector4D();
		else if (sType == "int")
			it->second.m_iValue = d.GetValueInt();
		else if (sType == "bool")
			it->second.m_bValue = d.GetValueBool();
		else if (sType == "mat4")
		{
			TUnimplemented(); 
		}
		else if (sType == "sampler2D")
		{
			TUnimplemented(); // Can't set a default texture... yet.
		}
		else
			TUnimplemented();
	}

	return true;
}

void CShader::Destroy()
{
	glDetachShader((GLuint)m_iProgram, (GLuint)m_iVShader);
	glDetachShader((GLuint)m_iProgram, (GLuint)m_iFShader);
	glDeleteShader((GLuint)m_iVShader);
	glDeleteShader((GLuint)m_iFShader);
	glDeleteProgram((GLuint)m_iProgram);
}

void ReloadShaders(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	CShaderLibrary::CompileShaders();
	if (CShaderLibrary::Get()->IsCompiled())
		TMsg("Shaders reloaded.\n");
	else
		TMsg("Shaders compile failed. See shaders.txt\n");
}

CCommand shaders_reload("shaders_reload", ::ReloadShaders);

#ifndef DT_SHADERS_H
#define DT_SHADERS_H

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

#include <tstring.h>

class CShader
{
public:
							CShader(const tstring& sName, const tstring& sVertexFile, const tstring& sFragmentFile);

public:
	bool					Compile();
	void					Destroy();

public:
	eastl::string			m_sName;
	eastl::string			m_sVertexFile;
	eastl::string			m_sFragmentFile;
	size_t					m_iVShader;
	size_t					m_iFShader;
	size_t					m_iProgram;

	size_t					m_iPositionAttribute;
	size_t					m_iNormalAttribute;
	size_t					m_iTexCoordAttribute;
};

class CShaderLibrary
{
	friend class CShader;

public:
							CShaderLibrary();
							~CShaderLibrary();

public:
	static size_t			GetNumShaders() { return Get()->m_aShaders.size(); };

	static CShader*			GetShader(const tstring& sName);
	static CShader*			GetShader(size_t i);

	static void				AddShader(const tstring& sName, const tstring& sVertex, const tstring& sFragment);

	static size_t			GetProgram(const tstring& sName);

	static void				CompileShaders();
	static void				DestroyShaders();

	static bool				IsCompiled() { return Get()->m_bCompiled; };

	static tstring			GetShaderFunctions() { return Get()->m_sFunctions; }

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					ClearLog();
	void					WriteLog(const char* pszLog, const char* pszShaderText);

protected:
	eastl::map<tstring, size_t>	m_aShaderNames;
	eastl::vector<CShader>	m_aShaders;
	bool					m_bCompiled;

	size_t					m_iTerrain;
	size_t					m_iModel;
	size_t					m_iProp;
	size_t					m_iScrollingTexture;
	size_t					m_iExplosion;
	size_t					m_iBlur;
	size_t					m_iBrightPass;
	size_t					m_iDarken;
	size_t					m_iStencil;
	size_t					m_iCameraGuided;

	bool					m_bLogNeedsClearing;

	tstring					m_sFunctions;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};

/*
	struct gl_LightSourceParameters {
		vec4 ambient; 
		vec4 diffuse; 
		vec4 specular; 
		vec4 position; 
		vec4 halfVector; 
		vec3 spotDirection; 
		float spotExponent; 
		float spotCutoff; // (range: [0.0,90.0], 180.0)
		float spotCosCutoff; // (range: [1.0,0.0],-1.0)
		float constantAttenuation; 
		float linearAttenuation; 
		float quadraticAttenuation;	
	};

	uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];

	struct gl_LightModelParameters {
		vec4 ambient; 
	};

	uniform gl_LightModelParameters gl_LightModel;

	struct gl_MaterialParameters {
		vec4 emission;   
		vec4 ambient;    
		vec4 diffuse;    
		vec4 specular;   
		float shininess; 
	};

	uniform gl_MaterialParameters gl_FrontMaterial;
	uniform gl_MaterialParameters gl_BackMaterial;
*/

#endif

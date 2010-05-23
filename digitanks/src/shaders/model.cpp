#include "shaders.h"

const char* CShaderLibrary::GetVSModelShader()
{
	return
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSModelShader()
{
	return
		"uniform bool bDiffuse;"
		"uniform sampler2D iDiffuse;"

		"uniform float flAlpha;"

		"void main()"
		"{"
		"	vec4 vecDiffuse = gl_Color;"

		"	if (bDiffuse)"
		"		vecDiffuse *= texture2D(iDiffuse, gl_TexCoord[0].st);"

		"	gl_FragColor = vecDiffuse;"
		"	gl_FragColor.a *= flAlpha;"
		"}";
}

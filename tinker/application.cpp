#include "application.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <time.h>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <iostream>
#include <fstream>

#include <strutils.h>
#include <tinker_platform.h>
#include <mtrand.h>
#include <tinker/keys.h>
#include <tinker/portals/portal.h>
#include <tinker/cvar.h>
#include <glgui/rootpanel.h>

CApplication* CApplication::s_pApplication = NULL;

CApplication::CApplication(int argc, char** argv)
{
	ilInit();

	TPortal_Startup();

	s_pApplication = this;

	srand((unsigned int)time(NULL));
	mtsrand((size_t)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_bIsOpen = false;

	m_bMultisampling = false;

	m_pConsole = NULL;
}

#ifdef _DEBUG
#define GL_DEBUG_VALUE "1"
#else
#define GL_DEBUG_VALUE "0"
#endif

CVar gl_debug("gl_debug", GL_DEBUG_VALUE);

void CALLBACK GLDebugCallback(GLenum iSource, GLenum iType, GLuint id, GLenum iSeverity, GLsizei iLength, const GLchar* pszMessage, GLvoid* pUserParam)
{
	if (iType != GL_DEBUG_TYPE_PERFORMANCE_ARB)
	{
		TAssert(iSeverity != GL_DEBUG_SEVERITY_HIGH_AMD);
		TAssert(iSeverity != GL_DEBUG_SEVERITY_MEDIUM_AMD);
	}

	if (gl_debug.GetBool())
	{
		TMsg("OpenGL Debug Message (");

		if (iSource == GL_DEBUG_SOURCE_API_ARB)
			TMsg("Source: API ");
		else if (iSource == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
			TMsg("Source: Window System ");
		else if (iSource == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
			TMsg("Source: Shader Compiler ");
		else if (iSource == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
			TMsg("Source: Third Party ");
		else if (iSource == GL_DEBUG_SOURCE_APPLICATION_ARB)
			TMsg("Source: Application ");
		else if (iSource == GL_DEBUG_SOURCE_OTHER_ARB)
			TMsg("Source: Other ");

		if (iType == GL_DEBUG_TYPE_ERROR_ARB)
			TMsg("Type: Error ");
		else if (iType == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
			TMsg("Type: Deprecated Behavior ");
		else if (iType == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
			TMsg("Type: Undefined Behavior ");
		else if (iType == GL_DEBUG_TYPE_PORTABILITY_ARB)
			TMsg("Type: Portability ");
		else if (iType == GL_DEBUG_TYPE_PERFORMANCE_ARB)
			TMsg("Type: Performance ");
		else if (iType == GL_DEBUG_TYPE_OTHER_ARB)
			TMsg("Type: Other ");

		if (iSeverity == GL_DEBUG_SEVERITY_HIGH_ARB)
			TMsg("Severity: High) ");
		else if (iSeverity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
			TMsg("Severity: Medium) ");
		else if (iSeverity == GL_DEBUG_SEVERITY_LOW_ARB)
			TMsg("Severity: Low) ");

		TMsg(convertstring<GLchar, tchar>(pszMessage) + "\n");
	}
}

void CApplication::OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable)
{
	glfwInit();

	m_bFullscreen = bFullscreen;

	if (HasCommandLineSwitch("--fullscreen"))
		m_bFullscreen = true;

	if (HasCommandLineSwitch("--windowed"))
		m_bFullscreen = false;

	m_iWindowWidth = iWidth;
	m_iWindowHeight = iHeight;

	glfwOpenWindowHint(GLFW_WINDOW_RESIZABLE, bResizeable?GL_TRUE:GL_FALSE);

	if (m_bMultisampling)
		glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

	if (HasCommandLineSwitch("--debug-gl"))
		glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwOpenWindowHint(GLFW_DEPTH_BITS, 16);
	glfwOpenWindowHint(GLFW_RED_BITS, 8);
	glfwOpenWindowHint(GLFW_GREEN_BITS, 8);
	glfwOpenWindowHint(GLFW_BLUE_BITS, 8);
	glfwOpenWindowHint(GLFW_ALPHA_BITS, 8);

	TMsg(sprintf(tstring("Opening %dx%d %s %s window.\n"), iWidth, iHeight, bFullscreen?"fullscreen":"windowed", bResizeable?"resizeable":"fixed-size"));

	if (!(m_pWindow = (size_t)glfwOpenWindow(iWidth, iHeight, m_bFullscreen?GLFW_FULLSCREEN:GLFW_WINDOWED, WindowTitle().c_str(), NULL)))
	{
		glfwTerminate();
		return;
	}

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (!m_bFullscreen)
	{
		// The taskbar is at the bottom of the screen. Pretend the screen is smaller so the window doesn't clip down into it.
		// Also the window's title bar at the top takes up space.
		iScreenHeight -= 70;

		int iWindowX = (int)(iScreenWidth/2-m_iWindowWidth/2);
		int iWindowY = (int)(iScreenHeight/2-m_iWindowHeight/2);
		glfwSetWindowPos((GLFWwindow)m_pWindow, iWindowX, iWindowY);
	}

	glfwSetWindowCloseCallback(&CApplication::WindowCloseCallback);
	glfwSetWindowSizeCallback(&CApplication::WindowResizeCallback);
	glfwSetKeyCallback(&CApplication::KeyEventCallback);
	glfwSetCharCallback(&CApplication::CharEventCallback);
	glfwSetMousePosCallback(&CApplication::MouseMotionCallback);
	glfwSetMouseButtonCallback(&CApplication::MouseInputCallback);
	glfwSetScrollCallback(&CApplication::MouseWheelCallback);
	glfwSwapInterval( 1 );
	glfwSetTime( 0.0 );

	SetMouseCursorEnabled(true);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	DumpGLInfo();

	if (GLEW_ARB_debug_output)
	{
		glDebugMessageCallbackARB(GLDebugCallback, nullptr);

		tstring sMessage("OpenGL Debug Output Activated");
		glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_LOW_ARB, sMessage.length(), sMessage.c_str());
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0);

	m_bIsOpen = true;
}

CApplication::~CApplication()
{
	glfwTerminate();

	TPortal_Shutdown();
}

#define MAKE_PARAMETER(name) \
{ #name, name } \

void CApplication::DumpGLInfo()
{
	glewInit();

	std::ifstream i(convertstring<tchar, char>(GetAppDataDirectory(AppDirectory(), "glinfo.txt")).c_str());
	if (i)
		return;
	i.close();

	std::ofstream o(convertstring<tchar, char>(GetAppDataDirectory(AppDirectory(), "glinfo.txt")).c_str());
	if (!o || !o.is_open())
		return;

	o << "Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;
	o << "Renderer: " << (char*)glGetString(GL_RENDERER) << std::endl;
	o << "Version: " << (char*)glGetString(GL_VERSION) << std::endl;

	char* pszShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (pszShadingLanguageVersion)
		o << "Shading Language Version: " << pszShadingLanguageVersion << std::endl;

	eastl::string sExtensions = (char*)glGetString(GL_EXTENSIONS);
	eastl::vector<eastl::string> asExtensions;
	strtok(sExtensions, asExtensions);
	o << "Extensions:" << std::endl;
	for (size_t i = 0; i < asExtensions.size(); i++)
		o << "\t" << asExtensions[i].c_str() << std::endl;

	typedef struct
	{
		const char* pszName;
		int iParameter;
	} GLParameter;

	GLParameter aParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_ATTRIB_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_CLIP_PLANES),
		MAKE_PARAMETER(GL_MAX_LIGHTS),
		MAKE_PARAMETER(GL_MAX_COLOR_MATRIX_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_MODELVIEW_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_PROJECTION_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_TEXTURE_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_3D_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB),
		MAKE_PARAMETER(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_VERTICES),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_INDICES),
		MAKE_PARAMETER(GL_MAX_EVAL_ORDER),
		MAKE_PARAMETER(GL_MAX_LIST_NESTING),
		MAKE_PARAMETER(GL_MAX_NAME_STACK_DEPTH),
		MAKE_PARAMETER(GL_MAX_PIXEL_MAP_TABLE),
		MAKE_PARAMETER(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_UNITS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_LOD_BIAS_EXT),
		MAKE_PARAMETER(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT),
		MAKE_PARAMETER(GL_MAX_DRAW_BUFFERS_ARB),

		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_VARYING_FLOATS_ARB),
		MAKE_PARAMETER(GL_MAX_VERTEX_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_COORDS_ARB),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_COORDS_ARB),
		MAKE_PARAMETER(GL_MAX_TEXTURE_IMAGE_UNITS_ARB),
	};

	// Clear it
	glGetError();

	o << std::endl;

	for (size_t i = 0; i < sizeof(aParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetIntegerv(aParameters[i].iParameter, &iValue);

		if (glGetError() != GL_NO_ERROR)
			continue;

		o << aParameters[i].pszName << ": " << iValue << std::endl;
	}

	GLParameter aProgramParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_PROGRAM_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEMPORARIES_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ENV_PARAMETERS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB),
		MAKE_PARAMETER(GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB),
	};

	if (!GLEW_ARB_vertex_program && !GLEW_NV_vertex_program)
		return;

	o << std::endl;
	o << "Vertex programs:" << std::endl;

	for (size_t i = 0; i < sizeof(aProgramParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, aProgramParameters[i].iParameter, &iValue);

		if (glGetError() == GL_NO_ERROR)
			o << aProgramParameters[i].pszName << ": " << iValue << std::endl;
	}

	o << std::endl;
	o << "Fragment programs:" << std::endl;

	for (size_t i = 0; i < sizeof(aProgramParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue;
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, aProgramParameters[i].iParameter, &iValue);

		if (glGetError() == GL_NO_ERROR)
			o << aProgramParameters[i].pszName << ": " << iValue << std::endl;
	}
}

void CApplication::SwapBuffers()
{
	glfwSwapBuffers();
	glfwPollEvents();
}

float CApplication::GetTime()
{
	return (float)glfwGetTime();
}

bool CApplication::IsOpen()
{
	return !!glfwIsWindow((GLFWwindow)m_pWindow) && m_bIsOpen;
}

void Quit(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	CApplication::Get()->Close();
}

CCommand quit("quit", ::Quit);

void CApplication::Close()
{
	m_bIsOpen = false;
}

bool CApplication::HasFocus()
{
	return glfwGetWindowParam((GLFWwindow)m_pWindow, GLFW_ACTIVE) == GL_TRUE;
}

void CApplication::Render()
{
}

int CApplication::WindowClose()
{
	return GL_TRUE;
}

void CApplication::WindowResize(int w, int h)
{
	m_iWindowWidth = w;
	m_iWindowHeight = h;

	Render();

	SwapBuffers();
}

void CApplication::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CApplication::MouseInput(int iButton, int iState)
{
	int mx, my;
	GetMousePosition(mx, my);
	if (iState == 1)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
		{
			m_bMouseDownInGUI = true;
			return;
		}
		else
			m_bMouseDownInGUI = false;
	}
	else
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return;

		if (m_bMouseDownInGUI)
		{
			m_bMouseDownInGUI = false;
			return;
		}
	}
}

tinker_keys_t MapKey(int c)
{
	switch (c)
	{
	case GLFW_KEY_ESC:
		return TINKER_KEY_ESCAPE;

	case GLFW_KEY_F1:
		return TINKER_KEY_F1;

	case GLFW_KEY_F2:
		return TINKER_KEY_F2;

	case GLFW_KEY_F3:
		return TINKER_KEY_F3;

	case GLFW_KEY_F4:
		return TINKER_KEY_F4;

	case GLFW_KEY_F5:
		return TINKER_KEY_F5;

	case GLFW_KEY_F6:
		return TINKER_KEY_F6;

	case GLFW_KEY_F7:
		return TINKER_KEY_F7;

	case GLFW_KEY_F8:
		return TINKER_KEY_F8;

	case GLFW_KEY_F9:
		return TINKER_KEY_F9;

	case GLFW_KEY_F10:
		return TINKER_KEY_F10;

	case GLFW_KEY_F11:
		return TINKER_KEY_F11;

	case GLFW_KEY_F12:
		return TINKER_KEY_F12;

	case GLFW_KEY_UP:
		return TINKER_KEY_UP;

	case GLFW_KEY_DOWN:
		return TINKER_KEY_DOWN;

	case GLFW_KEY_LEFT:
		return TINKER_KEY_LEFT;

	case GLFW_KEY_RIGHT:
		return TINKER_KEY_RIGHT;

	case GLFW_KEY_LSHIFT:
		return TINKER_KEY_LSHIFT;

	case GLFW_KEY_RSHIFT:
		return TINKER_KEY_RSHIFT;

	case GLFW_KEY_LCTRL:
		return TINKER_KEY_LCTRL;

	case GLFW_KEY_RCTRL:
		return TINKER_KEY_RCTRL;

	case GLFW_KEY_LALT:
		return TINKER_KEY_LALT;

	case GLFW_KEY_RALT:
		return TINKER_KEY_RALT;

	case GLFW_KEY_TAB:
		return TINKER_KEY_TAB;

	case GLFW_KEY_ENTER:
		return TINKER_KEY_ENTER;

	case GLFW_KEY_BACKSPACE:
		return TINKER_KEY_BACKSPACE;

	case GLFW_KEY_INSERT:
		return TINKER_KEY_INSERT;

	case GLFW_KEY_DEL:
		return TINKER_KEY_DEL;

	case GLFW_KEY_PAGEUP:
		return TINKER_KEY_PAGEUP;

	case GLFW_KEY_PAGEDOWN:
		return TINKER_KEY_PAGEDOWN;

	case GLFW_KEY_HOME:
		return TINKER_KEY_HOME;

	case GLFW_KEY_END:
		return TINKER_KEY_END;

	case GLFW_KEY_KP_0:
		return TINKER_KEY_KP_0;

	case GLFW_KEY_KP_1:
		return TINKER_KEY_KP_1;

	case GLFW_KEY_KP_2:
		return TINKER_KEY_KP_2;

	case GLFW_KEY_KP_3:
		return TINKER_KEY_KP_3;

	case GLFW_KEY_KP_4:
		return TINKER_KEY_KP_4;

	case GLFW_KEY_KP_5:
		return TINKER_KEY_KP_5;

	case GLFW_KEY_KP_6:
		return TINKER_KEY_KP_6;

	case GLFW_KEY_KP_7:
		return TINKER_KEY_KP_7;

	case GLFW_KEY_KP_8:
		return TINKER_KEY_KP_8;

	case GLFW_KEY_KP_9:
		return TINKER_KEY_KP_9;

	case GLFW_KEY_KP_DIVIDE:
		return TINKER_KEY_KP_DIVIDE;

	case GLFW_KEY_KP_MULTIPLY:
		return TINKER_KEY_KP_MULTIPLY;

	case GLFW_KEY_KP_SUBTRACT:
		return TINKER_KEY_KP_SUBTRACT;

	case GLFW_KEY_KP_ADD:
		return TINKER_KEY_KP_ADD;

	case GLFW_KEY_KP_DECIMAL:
		return TINKER_KEY_KP_DECIMAL;

	case GLFW_KEY_KP_EQUAL:
		return TINKER_KEY_KP_EQUAL;

	case GLFW_KEY_KP_ENTER:
		return TINKER_KEY_KP_ENTER;
	}

	if (c < 256)
		return (tinker_keys_t)TranslateKeyToQwerty(c);

	return TINKER_KEY_UKNOWN;
}

tinker_keys_t MapMouseKey(int c)
{
	switch (c)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		return TINKER_KEY_MOUSE_LEFT;

	case GLFW_MOUSE_BUTTON_RIGHT:
		return TINKER_KEY_MOUSE_RIGHT;

	case GLFW_MOUSE_BUTTON_MIDDLE:
		return TINKER_KEY_MOUSE_MIDDLE;
	}

	return TINKER_KEY_UKNOWN;
}

tinker_keys_t MapJoystickKey(int c)
{
	switch (c)
	{
	case GLFW_JOYSTICK_1:
		return TINKER_KEY_JOYSTICK_1;

	case GLFW_JOYSTICK_2:
		return TINKER_KEY_JOYSTICK_2;

	case GLFW_JOYSTICK_3:
		return TINKER_KEY_JOYSTICK_3;

	case GLFW_JOYSTICK_4:
		return TINKER_KEY_JOYSTICK_4;

	case GLFW_JOYSTICK_5:
		return TINKER_KEY_JOYSTICK_5;

	case GLFW_JOYSTICK_6:
		return TINKER_KEY_JOYSTICK_6;

	case GLFW_JOYSTICK_7:
		return TINKER_KEY_JOYSTICK_7;

	case GLFW_JOYSTICK_8:
		return TINKER_KEY_JOYSTICK_8;

	case GLFW_JOYSTICK_9:
		return TINKER_KEY_JOYSTICK_9;

	case GLFW_JOYSTICK_10:
		return TINKER_KEY_JOYSTICK_10;
	}

	return TINKER_KEY_UKNOWN;
}

void CApplication::MouseInputCallback(void*, int iButton, int iState)
{
	Get()->MouseInput(MapMouseKey(iButton), iState);
}

void CApplication::KeyEvent(int c, int e)
{
	if (e == GLFW_PRESS)
		KeyPress(MapKey(c));
	else
		KeyRelease(MapKey(c));
}

void CApplication::CharEvent(int c)
{
	if (c == '`')
	{
		ToggleConsole();
		return;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;

	DoCharPress(c);
}

bool CApplication::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c, IsCtrlDown()))
		return true;

	if (c == TINKER_KEY_F4 && IsAltDown())
		exit(0);

	return DoKeyPress(c);
}

void CApplication::KeyRelease(int c)
{
	DoKeyRelease(c);
}

bool CApplication::IsCtrlDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LCTRL) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RCTRL);
}

bool CApplication::IsAltDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LALT) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RALT);
}

bool CApplication::IsShiftDown()
{
	return glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_LSHIFT) || glfwGetKey((GLFWwindow)m_pWindow, GLFW_KEY_RSHIFT);
}

bool CApplication::IsMouseLeftDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

bool CApplication::IsMouseRightDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

bool CApplication::IsMouseMiddleDown()
{
	return glfwGetMouseButton((GLFWwindow)m_pWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

void CApplication::GetMousePosition(int& x, int& y)
{
	glfwGetMousePos((GLFWwindow)m_pWindow, &x, &y);
}

void CApplication::SetMouseCursorEnabled(bool bEnabled)
{
	if (bEnabled)
		glfwSetCursorMode( (GLFWwindow)m_pWindow, GLFW_CURSOR_NORMAL );
	else
		glfwSetCursorMode( (GLFWwindow)m_pWindow, GLFW_CURSOR_CAPTURED );

	m_bMouseEnabled = bEnabled;
}

bool CApplication::IsMouseCursorEnabled()
{
	return m_bMouseEnabled;
}

bool CApplication::HasCommandLineSwitch(const char* pszSwitch)
{
	for (size_t i = 0; i < m_apszCommandLine.size(); i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return true;
	}

	return false;
}

const char* CApplication::GetCommandLineSwitchValue(const char* pszSwitch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_apszCommandLine.size()-1; i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return m_apszCommandLine[i+1];
	}

	return NULL;
}

void CreateApplicationWithErrorHandling(CreateApplicationCallback pfnCallback, int argc, char** argv)
{
#ifdef _WIN32
#ifndef _DEBUG
	__try
	{
#endif
#endif

		// Put in a different function to avoid warnings and errors associated with object deconstructors and try/catch blocks.
		pfnCallback(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation(), "Tinker"), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}

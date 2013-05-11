#include "reflection_window.h"

void CreateApplication(int argc, char** argv)
{
	CReflectionWindow oWindow(argc, argv);

	oWindow.OpenWindow();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}

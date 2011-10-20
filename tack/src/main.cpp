#include "tack_window.h"

void CreateApplication(int argc, char** argv)
{
	CTackWindow oWindow(argc, argv);

	oWindow.OpenWindow();
	oWindow.SetupEngine();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}

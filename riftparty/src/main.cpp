#include "riftparty_window.h"

void CreateApplication(int argc, char** argv)
{
	CRiftPartyWindow oWindow(argc, argv);

	oWindow.OpenWindow();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}

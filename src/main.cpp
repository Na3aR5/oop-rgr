#include <windows.h>
#include <rgr/Application.h>

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
	AllocConsole();
	(void)freopen("CONOUT$", "w", stdout);

	rgr::Application app{};
	app.Run();

	return 0;
}
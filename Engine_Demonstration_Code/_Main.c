#include"Windows.h"
#include"Project_All.h"

void main()
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	MoonProjectInit((char*)"New Project", -1, -1, 800, 600, 100, _Resource);
	MoonProjectRun(SettingAll, GameLogicAll, GameDrawAll);
	MoonProjectOver(SettingOver);
}
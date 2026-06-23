#include"Windows.h"
#include"Project_All.h"

void main()
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	static MOON_PROJECTGOD project;
	MoonProjectInit(&project, (char*)"New Project", -1, -1, 800, 600, 100, _Resource);
	MoonProjectRun(&project, SettingAll, GameLogicAll, GameDrawAll);
	MoonProjectOver(&project, SettingOver);
}
#include"Project_All.h"

void main()
{
	MoonCMD(MOON_ON);
	static MOON_PROJECTGOD project;
	MoonProjectInit(&project, (char*)"MoonEngine_OpenGL", -1, -1, 800, 600, 100, _Resource);
	MoonProjectRun(&project, SettingAll, GameLogicLoad, GameDrawLoad);
	MoonProjectOver(&project, SettingOver);
}
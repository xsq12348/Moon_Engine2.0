#include"Project_All.h"

static MOON_TIMELOAD logictps;

extern MOON_PROJECTSETTING(_Resource)
{
	MoonCreateEntityIndex(project, &logictps, (char*)"LogicTps", sizeof(MOON_TIMELOAD), (char*)"MOON_TIMELOAD");
	
	MoonProjectFindEntityAllNumber(project);

}
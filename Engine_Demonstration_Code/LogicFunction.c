#include"Project_All.h"

static MOON_TIMELOAD* logictps;
static POINT* mousecoord;

/*
* 函數 HardwareProcess
* 作用 硬件檢測
*/
static void HardwareProcess();

extern MOON_PROJECTMODULE(GameLogicLoad)
{
	MoonHashFindEntity(project, "LogicTps", MOON_TIMELOAD, logictps_2);
	logictps = logictps_2;

	MoonHashFindEntity(project, "ProjectMouseCoord", POINT, mousecoord_2);
	mousecoord = mousecoord_2;

	MOON_METADATA metadata;
	metadata.function = GameLogicAll;
	MoonProjectSendMessage(MOON_MESSAGE_LOGIC_SETLOGIC, metadata);
	
	return 1;
}

extern MOON_PROJECTMODULE(GameLogicAll)
{
	//帧限制
	if (!MoonTimeLoad(logictps, TRUE))
	{
		MoonSleep(1);
		return 1;
	}

	HardwareProcess();
	return 1;
}

static void HardwareProcess()
{

}
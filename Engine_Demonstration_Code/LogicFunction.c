#include"Project_All.h"

static MOON_TIMELOAD* logictps;
static MOON_POINT2D* mousecoord;

/*
* 函數 HardwareProcess
* 作用 硬件檢測
*/
static void HardwareProcess();

extern MOON_PROJECTMODULE(GameLogicLoad)
{
	MoonHashFindEntity("LogicTps", MOON_TIMELOAD, logictps_2);
	logictps = logictps_2;

	MoonHashFindEntity("ProjectMouseCoord", MOON_POINT2D, mousecoord_2);
	mousecoord = mousecoord_2;
	
	return 1;
}

extern MOON_PROJECTMODULE(GameLogicAll)
{
	//帧限制
	if (!MoonTimeLoad(logictps, MOON_TRUE))
	{
		MoonSleep(1);
		return 1;
	}

	//按下A切换模块DrawAll
	if (MoonKeyState(MOON_KEY_A))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameDrawAll;
		MoonProjectSendMessage(MOON_MESSAGE_SETDRAW, metadata);
	}

	//按下B切换模块DrawAll_2
	if (MoonKeyState(MOON_KEY_B))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameDrawAll_2;
		MoonProjectSendMessage(MOON_MESSAGE_SETDRAW, metadata);
	}

	//按下C切换模块LogicAll_2
	if (MoonKeyState(MOON_KEY_A))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameLogicAll_2;
		MoonProjectSendMessage(MOON_MESSAGE_SETLOGIC, metadata);
	}


	HardwareProcess();
	return 1;
}

extern MOON_PROJECTMODULE(GameLogicAll_2)
{
	//帧限制
	if (!MoonTimeLoad(logictps, MOON_TRUE))
	{
		MoonSleep(1);
		return 1;
	}

	//按下A切换模块DrawAll_2
	if (MoonKeyState(MOON_KEY_A))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameDrawAll_2;
		MoonProjectSendMessage(MOON_MESSAGE_SETDRAW, metadata);
	}

	//按下B切换模块DrawAll
	if (MoonKeyState(MOON_KEY_B))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameDrawAll;
		MoonProjectSendMessage(MOON_MESSAGE_SETDRAW, metadata);
	}

	//按下C切换模块LogicAll
	if (MoonKeyState(MOON_KEY_A))
	{
		GameDrawLoad();	//加载绘制模块
		MOON_METADATA metadata = { 0 };
		metadata.function = GameLogicAll;
		MoonProjectSendMessage(MOON_MESSAGE_SETLOGIC, metadata);
	}


	HardwareProcess();
	return 1;
}
static void HardwareProcess()
{

}
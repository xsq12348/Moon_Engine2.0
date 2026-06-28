#include"Project_All.h"

static MOON_IMAGE* engineback;
static MOON_POINT2D* mousecoord;
static int* fps;

extern MOON_PROJECTMODULE(GameDrawLoad)
{
	MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, engineback_2);
	engineback = engineback_2;

	MoonHashFindEntity("ProjectMouseCoord", MOON_POINT2D, mousecoord_2);
	mousecoord = mousecoord_2;
	
	MoonHashFindEntity((char*)"ProjectFPS", int, fpsnumber);
	fps = fpsnumber;

    return 1;
}

extern MOON_PROJECTMODULE(GameDrawAll)
{
	//切换背景颜色
	{
		MOON_METADATA metadata = { 0 };
		metadata.draw.image_goal = engineback;
		metadata.draw.color = MoonRGBA(125, 200, 255, 255);
		MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_CLEAN, metadata);
	}

	//绘制白色方块
	MoonDrawBoxFull(engineback, 0, 0, 100, 100, MoonRGBA(255, 255, 255, 255));
	return 1;
}

extern MOON_PROJECTMODULE(GameDrawAll_2)
{	
	//切换背景颜色
	{
		MOON_METADATA metadata = { 0 };
		metadata.draw.image_goal = engineback;
		metadata.draw.color = MoonRGBA(125, 200, 255, 255);
		MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_CLEAN, metadata);
	}
	//绘制灰色方块
	MoonDrawBoxFull(engineback, 0, 0, 100, 100, MoonRGBA(100, 100, 100, 255));
}
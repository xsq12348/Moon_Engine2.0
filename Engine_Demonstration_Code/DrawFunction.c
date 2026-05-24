#include"Project_All.h"

static MOON_IMAGE* engineback;
static MOON_POINT2D* mousecoord;
static int* fps;

extern MOON_PROJECTMODULE(GameDrawLoad)
{
	MoonHashFindEntity(project, "ProjectBitmap", MOON_IMAGE, engineback_2);
	engineback = engineback_2;

	MoonHashFindEntity(project, "ProjectMouseCoord", MOON_POINT2D, mousecoord_2);
	mousecoord = mousecoord_2;
	
	MoonHashFindEntity(project, (char*)"ProjectFPS", int, fpsnumber);
	fps = fpsnumber;

	MOON_METADATA metadata;
	metadata.function = GameDrawAll;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_SETDRAW, metadata);
    printf("GLFW Version: %s\n", glfwGetVersionString());
    return 1;
}

extern MOON_PROJECTMODULE(GameDrawAll)
{

	return 1;
}
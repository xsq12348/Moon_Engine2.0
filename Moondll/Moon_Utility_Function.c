#include"MoonCore.h"

//临时备忘录,发布时需要删除
//需要提供一个直接读取文本文件的函数,用于加载shader

MOON_PROJECTGOD* utility_project;

_declspec(dllexport) extern void MoonUtilityLoad(MOON_PROJECTGOD* project)
{
	utility_project = project;
}

_declspec(dllexport) extern void MoonMusic(const char* File)
{
	//SDL_OpenAudioDeviceStream();
	//TCHAR cmd[255];
	//wsprintf(cmd, TEXT("open \%s\ alias music"), File);
	//mciSendString(TEXT("close music"), 0, 0, 0);
	//mciSendString(cmd, MOON_NULL, 0, MOON_NULL);
	//mciSendString(TEXT("play music"), MOON_NULL, 0, MOON_NULL);
}

_declspec(dllexport) extern int MoonSleep(int timeload)
{
	SDL_Delay(timeload);
	return 0;
}

_declspec(dllexport) extern unsigned int MoonHash(char* text)
{
	if (text == MOON_NULL)return MOON_Error;
	unsigned int length = (unsigned)strlen(text), hash = 0;
	if (length <= 0)return MOON_Error;
	for (unsigned int index = 0; index < length; index++)hash += text[index] * (index + 1);
	return hash;
}

_declspec(dllexport) extern void MoonTimeLoadInit(MOON_TIMELOAD* Timeload, int load)
{
	Timeload->time1 = 0;
	Timeload->time2 = 0;
	Timeload->timeload = load;
	Timeload->timeswitch = 0;
}

_declspec(dllexport) extern _Bool MoonKeyState(unsigned int Key)
{
	static unsigned char KEYSTATEbuffer[GLFW_KEY_LAST + 1];
	int state = glfwGetKey(utility_project->hwnd, Key) || glfwGetMouseButton(utility_project->hwnd, Key);
	if (!(state == GLFW_PRESS))KEYSTATEbuffer[Key] = 0;
	else if (KEYSTATEbuffer[Key] == 0) { KEYSTATEbuffer[Key] = MOON_TRUE; return MOON_TRUE; }
	return MOON_FALSE;
}

_declspec(dllexport) extern _Bool MoonKeyReal(unsigned int Key)
{
	return glfwGetKey(utility_project->hwnd, Key) || glfwGetMouseButton(utility_project->hwnd, Key);
}

_declspec(dllexport) extern int MoonTimeLoad(MOON_TIMELOAD* Timeload, int mode)
{
	if (!mode)return MOON_FALSE;
	else if (Timeload == MOON_NULL)
	{
		MoonPrompt("[TimeLoad函数错误!]存在空指针");
		return MOON_Error;
	}
	if (!Timeload->timeswitch)
	{
		Timeload->time1 = clock();
		if (Timeload->time1 > Timeload->time2 + Timeload->timeload)
		{
			Timeload->time2 = clock();
			Timeload->timeswitch = MOON_TRUE;
		}
	}
	else
	{
		Timeload->time2 = clock();
		Timeload->timeswitch = MOON_FALSE;
	}
	return Timeload->timeswitch;
}

_declspec(dllexport) extern void* MoonFindEntity(MOON_PROJECTGOD* project, char* nameid)
{
	void* entity = project->entityindex[(MoonHash(nameid) % ENTITYNUMBER)].entityindex;
	if (entity == MOON_NULL)
	{
		char text[255];
		snprintf(text, 255, "[MoonFindEntity]空指针错误!来自名称[%s]的实体", nameid);
		MoonProjectError(project->entityindex[(MoonHash(nameid) % ENTITYNUMBER)].entityindex, 1, text);
		return MOON_NULL;
	}
	return entity;
}

_declspec(dllexport) extern int MoonCreateEntityIndex(MOON_PROJECTGOD* project, void* arrentity, char* nameid, size_t size_len, char* type_name)
{
	int index = MOON_NOTFOUND;
	int hash = MoonHash(nameid) % ENTITYNUMBER;
	if (project->entityindex[hash].entityindex == MOON_NULL)
	{
		if (hash != MOON_Error)
		{
			index = hash;
			project->entityindex[index].entityindex = arrentity;
			project->entityindex[index].nameid = nameid;
			project->entityindex[index].length = (int)size_len;
			project->entityindex[index].type_name = type_name;
		}
		else printf("非法的名称[%s],无法通过这个字符串得到合法的索引", nameid);
	}
	else
	{
		printf("[CreateEntityIndex函数]报错,叫做[%s]的实体,此位置[%d],已有实体存在,名称为[%s],请换一个名字", nameid, hash, project->entityindex[index].nameid);
		strlen(project->entityindex[hash].nameid) <= 0 && MoonProjectError(&project->entityindex[hash], 2, (char*)"来自[CreateEntityIndex函数]的错误,出现了幽灵实体,没有合法名称");
		index = MOON_NOTFOUND;
	}
	char text[255];
	snprintf(text, 255, "创建了名称为[\033[31;46m%s\033[0m]的实体\n类型:%s\n地址:0x%p\nHash:%d\n索引:%d", nameid, project->entityindex[hash].type_name, project->entityindex[hash].entityindex, MoonHash(nameid), index);
	MoonPrompt(text);
	return index;
}

_declspec(dllexport) extern int MoonTriangleDetection(MOON_POINT2D a, MOON_POINT2D b, MOON_POINT2D c, MOON_POINT2D p)
{
	int d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
	int d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
	int d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);
	return (d1 * d2 > 0) && (d2 * d3 > 0);
}

_declspec(dllexport) extern int MoonButtonInit(MOON_BUTTON* button, int x, int y, int width, int height)
{
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->mode = MOON_FALSE;
	button->triggermode = 1;
	return 1;
}

_declspec(dllexport) extern int MoonButtonDetection(MOON_PROJECTGOD* project, char* name)
{
	MoonHashFindEntity(project, name, MOON_BUTTON, button);
	if (project->entityindex[(MoonHash(name) % ENTITYNUMBER)].length != sizeof(MOON_BUTTON))
	{
		MoonProjectError(button, 3, (char*)"[ButtonDetection函数]错误!错误原因:类型导入错误.");
		return 0;
	}
	MoonHashFindEntity(project, "ProjectMouseCoord", MOON_POINT2D, mousecoord);
	if (mousecoord->x > button->x && mousecoord->x < (button->x + button->width) && mousecoord->y > button->y && mousecoord->y < (button->y + button->height))
	{
		if (MoonKeyState(button->triggermode))button->mode = MOON_BUTTONPRESS;
		else button->mode = MOON_BUTTONRHOVER;

	}
	else button->mode = MOON_FALSE;
	switch (button->mode)
	{
	case MOON_BUTTONPRESS:	
		button->ButtonModePress && button->ButtonModePress(project, button);
			return MOON_BUTTONPRESS;
		break;
	case MOON_BUTTONRHOVER:
			button->ButtonModeHover && button->ButtonModeHover(project, button);
			return MOON_BUTTONRHOVER;
		break;
	}
	return 0;
}

_declspec(dllexport) extern int MoonButtonSetTriggerMode(MOON_PROJECTGOD* project,char* name,unsigned char key)
{
	MoonHashFindEntity(project, name, MOON_BUTTON, button);
	if (project->entityindex[MoonHash(name)].length != sizeof(MOON_BUTTON))
	{
		MoonProjectError(button, 3, (char*)"[ButtonSetTriggerMode函数]错误!错误原因:类型导入错误.");
		return 0;
	}
	button->triggermode = key;
	return key;
}

_declspec(dllexport) extern int MoonCharToWchar(wchar_t* text1, char* text2 , int len)
{
	setlocale(LC_ALL, "");
	mbstowcs(text1, text2, len);
	return 1;
}

_declspec(dllexport) extern int MoonWcharToChar(char* text1, wchar_t* text2, int len)
{
	setlocale(LC_ALL, "");
	wcstombs(text1, text2, len);
	return 1;
}

_declspec(dllexport) extern void MoonSetMouse(MOON_CURSOR_MODE mode)
{
	switch (mode)
	{
	case MOON_CURSOR_MODE_HIDDEN:
		glfwSetInputMode(utility_project->hwnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
		
	case MOON_CURSOR_MODE_DISABLED:
		glfwSetInputMode(utility_project->hwnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
		
	case MOON_CURSOR_MODE_CAPTURED:
		glfwSetInputMode(utility_project->hwnd, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
		break;
	}
	return;
}

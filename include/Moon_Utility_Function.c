#include"MoonCore.h"

MOON_PROJECTGOD* utility_project;

extern void MoonUtilityLoad(MOON_PROJECTGOD* project)
{
	utility_project = project;
}

extern void MoonMusic(const char* File)
{
	//SDL_OpenAudioDeviceStream();
	//TCHAR cmd[255];
	//wsprintf(cmd, TEXT("open \%s\ alias music"), File);
	//mciSendString(TEXT("close music"), 0, 0, 0);
	//mciSendString(cmd, MOON_NULL, 0, MOON_NULL);
	//mciSendString(TEXT("play music"), MOON_NULL, 0, MOON_NULL);
}

extern int MoonSleep(int timeload)
{
	SDL_Delay(timeload);
	return 0;
}

extern unsigned int MoonHash(char* text)
{
	if (text == MOON_NULL)return MOON_Error;
	unsigned int length = (unsigned)strlen(text), hash = 0;
	if (length <= 0)return MOON_Error;
	for (unsigned int index = 0; index < length; index++)hash += text[index] * (index + 1);
	return hash;
}

extern void MoonTimeLoadInit(MOON_TIMELOAD* Timeload, int load)
{
	Timeload->time1 = 0;
	Timeload->time2 = 0;
	Timeload->timeload = load;
	Timeload->timeswitch = 0;
}

extern _Bool MoonKeyState(unsigned int Key)
{
	static unsigned char KEYSTATEbuffer[GLFW_KEY_LAST + 1];
	int state = glfwGetKey(utility_project->hwnd, Key) || glfwGetMouseButton(utility_project->hwnd, Key);
	if (!(state == GLFW_PRESS))KEYSTATEbuffer[Key] = 0;
	else if (KEYSTATEbuffer[Key] == 0) { KEYSTATEbuffer[Key] = MOON_TRUE; return MOON_TRUE; }
	return MOON_FALSE;
}

extern _Bool MoonKeyReal(unsigned int Key)
{
	return glfwGetKey(utility_project->hwnd, Key) || glfwGetMouseButton(utility_project->hwnd, Key);
}

extern int MoonTimeLoad(MOON_TIMELOAD* Timeload, int mode)
{
	if (!mode)return MOON_FALSE;
	else if (Timeload == MOON_NULL)
	{
		MoonPrompt((char*)"[TimeLoad函数错误!]存在空指针");
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

extern void* MoonFindEntity(MOON_PROJECTGOD* project, char* nameid)
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

extern int MoonCreateEntityIndex(MOON_PROJECTGOD* project, void* arrentity, char* nameid, size_t size_len, char* type_name)
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

extern int MoonTriangleDetection(MOON_POINT2D a, MOON_POINT2D b, MOON_POINT2D c, MOON_POINT2D p)
{
	int d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
	int d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
	int d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);
	return (d1 * d2 > 0) && (d2 * d3 > 0);
}

extern int MoonButtonInit(MOON_BUTTON* button, int x, int y, int width, int height)
{
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->mode = MOON_FALSE;
	button->triggermode = 1;
	return 1;
}

extern int MoonButtonDetection(MOON_BUTTON* button, int x, int y, void* context)
{
	static _Bool mode = 0;
	if (
		button->x < x
		&& button->y < y
		&& button->x + button->width > x
		&& button->y + button->height > y
		)
	{
		if (MoonKeyState(button->triggermode))
		{
			mode = MOON_TRUE;
			button->mode = MOON_BUTTON_PRESS;
			if (button->ButtonModePress)
				button->ButtonModePress(button, context);
			return MOON_BUTTON_PRESS;
		}
		else
			if (mode && MoonKeyReal(button->triggermode))
			{
				if (button->ButtonModePressL)
					button->ButtonModePressL(button, context);
				button->mode = MOON_BUTTON_PRESS_LONG;
				return MOON_BUTTON_PRESS_LONG;
			}
			else
			{
				mode = MOON_FALSE;
				if (button->ButtonModeHover)
					button->ButtonModeHover(button, context);
				button->mode = MOON_BUTTON_RHOVER;
				return MOON_BUTTON_RHOVER;
			}
	}
	else
	{
		mode = MOON_FALSE;
		button->mode = MOON_BUTTON_FALSE;
		return MOON_BUTTON_FALSE;
	}
}

extern int MoonButtonSetTriggerMode(MOON_PROJECTGOD* project, MOON_BUTTON* button, unsigned int key)
{
	button->triggermode = key;
	return key;
}

extern int MoonCharToWchar(wchar_t* text1, char* text2 , int len)
{
	setlocale(LC_ALL, "");
	mbstowcs(text1, text2, len);
	return 1;
}

extern int MoonWcharToChar(char* text1, wchar_t* text2, int len)
{
	setlocale(LC_ALL, "");
	wcstombs(text1, text2, len);
	return 1;
}

extern void MoonSetMouse(MOON_CURSOR_MODE mode)
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

extern _Bool MoonFileLoad_TEXT(char* file_name, char* text, unsigned int text_size)
{
	for (unsigned int index = 0; index < text_size; index++)
		text[index] = '\0';

	FILE* fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		MoonPrompt((char*)"");
		printf("[FileLoad_TEXT]文件错误\n[%s]文件读取失败\n", file_name);
		return MOON_FALSE;
	}

	{
		int chbuffer[3] = { 0 };
		chbuffer[0] = fgetc(fp);
		chbuffer[1] = fgetc(fp);
		chbuffer[2] = fgetc(fp);

		if (chbuffer[0] == 0xEF
			&& chbuffer[1] == 0xBB
			&& chbuffer[2] == 0xBF)
		{
			MoonPrompt((char*)"");
			printf("\n[FileLoad_TEXT]检测到BOM开头的的特殊字节码 0xEF 0xBB 0xBF\n");
		}

		else
		{
			ungetc(chbuffer[2], fp);
			ungetc(chbuffer[1], fp);
			ungetc(chbuffer[0], fp);
		}
	}

	int ch = fgetc(fp);
	for (unsigned int index = 0; index < text_size; index++)
		if (ch != EOF)
		{
			text[index] = ch;
			ch = fgetc(fp);
		}
		else
			break;

	fclose(fp);

	if (text[text_size - 1] != '\0')
	{
		MoonPrompt((char*)"");
		printf("\n[FileLoad_TEXT]函数错误\n[%s]文件,传入的[text_size]空间不足,已清空字符串\n", file_name);
		for (unsigned int index = 0; index < text_size; index++)
			text[index] = '\0';
		return MOON_FALSE;
	}

	return MOON_TRUE;
}

extern unsigned int MoonStrMatch_Prefix(const char* str_1, const char* str_2)
{
	unsigned int index = 0;
	if (!str_1 || !str_2)
	{
		MoonPrompt((char*)"[MoonStrMatch_PrefixIgnore]函数提示,str_1或者str_2参数传入不合理");
		return MOON_FALSE;
	}

	while (str_1[index]
		&& str_1[index] == str_2[index])
		index++;
	return index;
}

extern unsigned int MoonStrMatch_PrefixIgnore(const char* str_1, const char* str_2, char ch)
{
	unsigned int
		index_1 = 0,
		index_2 = 0,
		index_all = 0;
	if (!str_1 || !str_2)
	{
		MoonPrompt((char*)"[MoonStrMatch_PrefixIgnore]函数提示,str_1或者str_2参数传入不合理");
		return MOON_FALSE;
	}

	while (str_1[index_1] && str_2[index_2])
	{
		while (str_1[index_1]
			&& str_1[index_1] == ch)
			index_1++;
		while (str_2[index_2]
			&& str_2[index_2] == ch)
			index_2++;
		if ((!str_1[index_1] || !str_2[index_2])
			|| str_1[index_1] != str_2[index_2])
			break;
		index_all++;
		index_1++;
		index_2++;
	}
	return index_all;
}

extern void MoonStrMatch_Replace(char* str, unsigned int start_index, unsigned int len, char ch_goal, char ch_replace)
{
	if (!str || strlen(str) <= start_index)
	{
		MoonPrompt((char*)"[MoonStrMatch_Replace] start_index参数传入不合理");
		return;
	}
	for (unsigned int index = start_index; str[index] && index < start_index + len; index++)
		if (str[index] == ch_goal)
			str[index] = ch_replace;
}

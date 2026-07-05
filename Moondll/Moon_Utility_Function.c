#include"Moon.h"
#include"MoonCore.h"

static MOON_ENGINECORE* utility_core;
static MOON_ALLOC_REGISTRY moon_alloc;
static MOON_POINT2D* moon_mouse_coord;
static MOON_CORE_MUSIC* moon_music_sound;
static unsigned int moon_music_index;
static SDL_AudioDeviceID moon_audio_dev = 0;
static SDL_AudioStream* moon_audio_stream = (SDL_AudioStream*)MOON_NULL;
static int* fps;
static unsigned char* moon_key;
static MOON_IMAGE* moon_engineback;

/*
* 函數 MoonAlloc_Registry
* 作用 内部注册表名单+1
* 使用方法
* MoonAlloc_Registry();
*/
static unsigned char MoonAlloc_Registry();

_declspec(dllexport) extern void MoonUtilityLoad(MOON_ENGINECORE* core)
{
	utility_core = core;
}

_declspec(dllexport) extern void MoonUtilityCoreLoad(MOON_ENGINECORE* core)
{
	{
		SDL_AudioSpec audio_spec;
		audio_spec.format = SDL_AUDIO_F32LE;
		audio_spec.channels = 1;
		audio_spec.freq = MOON_SAMPLE_RATE;

		{
			moon_audio_dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);
			if (moon_audio_dev == MOON_FALSE)
			{
				MoonPrompt((char*)"");
				printf("[ProjectInit] 打开音频设备失败: %s\n", SDL_GetError());
				MoonProjectDead();
				return;
			}
		}

		{
			moon_audio_stream = SDL_CreateAudioStream(&audio_spec, NULL);
			if (!moon_audio_stream)
			{
				MoonPrompt((char*)"");
				printf("[ProjectInit] 创建音频流失败: %s\n", SDL_GetError());
				SDL_CloseAudioDevice(moon_audio_dev);
				MoonProjectDead();
				return;
			}
		}

		{
			if (!SDL_BindAudioStream(moon_audio_dev, moon_audio_stream))
			{
				MoonPrompt((char*)"");
				printf("[ProjectInit] 绑定音频流失败: %s\n", SDL_GetError());
				SDL_DestroyAudioStream(moon_audio_stream);
				SDL_CloseAudioDevice(moon_audio_dev);
				MoonProjectDead();
				return;
			}
		}

		{
			SDL_ResumeAudioDevice(moon_audio_dev);
		}
	}

	{
		MoonHashFindEntity("ProjectMouseCoord", MOON_POINT2D, mousecoord_2);
		moon_mouse_coord = mousecoord_2;
	}

	{
		MoonHashFindEntity("ProjectFPS", int, fpsnumber);
		fps = fpsnumber;
	}

	{
		MoonHashFindEntity("ProjectKey", unsigned char, moon_key_2);
		moon_key = moon_key_2;
	}

	{
		MoonHashFindEntity("ProjectBitmap", MOON_IMAGE, engineback_2);
		moon_engineback = engineback_2;
	}
}

_declspec(dllexport) extern void MoonUtilityOver()
{
	for (unsigned int index = 0; index < moon_alloc.index; index++)
	{
		free(moon_alloc.alloc[index]);
		moon_alloc.alloc[index] = MOON_NULL;
	}
	free(moon_alloc.alloc);

	SDL_UnbindAudioStream(moon_audio_stream);
	SDL_DestroyAudioStream(moon_audio_stream);
	SDL_CloseAudioDevice(moon_audio_dev);
}

_declspec(dllexport) extern inline void MoonMusicSet(MOON_MUSIC* music,MOON_MUSIC_MODE on_or_off,float start,float end)
{
	music->mode = on_or_off;
	music->start = start;
	music->end = end;
}

_declspec(dllexport) extern inline void MoonMusicAgain(MOON_MUSIC* music)
{
	music->mode = MOON_MUSIC_MODE_AGAIN;
}

_declspec(dllexport) extern int MoonMusic(MOON_MUSIC* music)
{
	if (!music)
	{
		MoonPrompt((char*)"[MoonMusic] 非法的music指针");
		return MOON_FALSE;
	}

	if (music->mode == MOON_MUSIC_MODE_FALSE)
	{
		SDL_FlushAudioStream(moon_audio_stream);
		return MOON_FALSE;
	}

	if ((music->start >= music->end) || (music->start < 0.f || music->start > 1.f) || (music->end < 0.f || music->end > 1.f))
	{
		MoonPrompt((char*)"[MoonMusic] 非法的范围");
		return MOON_FALSE;
	}

	{
		if (music->id >= moon_music_index)
		{
			MoonPrompt((char*)"[MoonMusic] 无效的音乐ID");
			return MOON_FALSE;
		}

		if (!moon_music_sound[music->id].data || moon_music_sound[music->id].length <= 0)
		{
			MoonPrompt((char*)"[MoonMusic] 音效数据为空");
			return MOON_FALSE;
		}
		int avail = SDL_GetAudioStreamAvailable(moon_audio_stream);
		static unsigned int current_music_id = 0xffffffff;
		{
			switch (music->mode)
			{
			case MOON_MUSIC_MODE_AGAIN:
			{
				SDL_FlushAudioStream(moon_audio_stream);

				if (avail > 0)
				{
					unsigned char* dummy = (unsigned char*)malloc(avail);
					SDL_GetAudioStreamData(moon_audio_stream, dummy, avail);
					free(dummy);
				}

				current_music_id = music->id;
				music->mode = MOON_MUSIC_MODE_RUN;
			}
			break;
			case MOON_MUSIC_MODE_RUN:
			{
				unsigned int residual = SDL_GetAudioStreamQueued(moon_audio_stream);
				if (residual > 0)
				{
					//如果相同id且正在播放,那么退出
					if (current_music_id == music->id)
						return MOON_TRUE;
					//如果不同id,那么清空当前音乐

					if (avail > 0)
					{
						unsigned char* dummy = (unsigned char*)malloc(avail);
						SDL_GetAudioStreamData(moon_audio_stream, dummy, avail);
						free(dummy);
					}

					current_music_id = music->id;
				}
				else
				{
					if (music->id == current_music_id)
					{
						//如果播放完毕,那么直接退出
						current_music_id = 0xffffffff;
						music->mode = MOON_MUSIC_MODE_FALSE;
						return MOON_FALSE;
					}
					//首次播放
					current_music_id = music->id;
				}
			}
			break;
			}

			{
				MOON_CORE_MUSIC* current_music = &moon_music_sound[current_music_id];
				int len = (int)((music->end - music->start) * current_music->length) * sizeof(float);
				float* start = current_music->data + (int)(music->start * current_music->length);
				if (!SDL_PutAudioStreamData(moon_audio_stream, start, len))
				{
					MoonPrompt((char*)"[MoonMusic] 推入音频数据失败");
					printf("[MoonMusic] SDL 错误: %s\n", SDL_GetError());
					return MOON_FALSE;
				}
			}
		}
	}

	return MOON_TRUE;
}

_declspec(dllexport) extern unsigned char MoonMusicInit_Wav(MOON_MUSIC* music, const char* File)
{
	if (!music)
	{
		MoonPrompt((char*)"[MoonMusic] 非法的music指针");
		return MOON_FALSE;
	}
	if (!File)
	{
		MoonPrompt((char*)"[MoonMusicInit] 非法的File指针");
		return MOON_FALSE;
	}
	music->start = 0.f;
	music->end = 1.f;
	music->mode = MOON_MUSIC_MODE_FALSE;

	unsigned char* raw_data = (unsigned char*)MOON_NULL;
	unsigned int raw_len = MOON_FALSE;

	SDL_AudioSpec file_spec;

	if (!SDL_LoadWAV(File, &file_spec, &raw_data, &raw_len)) 
	{
		MoonPrompt((char*)"");
		printf("[MoonMusicInit] 加载失败: %s\n", SDL_GetError());
		return MOON_FALSE;
	}

	{
		SDL_AudioSpec device_spec;

		device_spec.format = SDL_AUDIO_F32LE;
		device_spec.channels = 1;
		device_spec.freq = MOON_SAMPLE_RATE;

		SDL_AudioStream* converter = SDL_CreateAudioStream(&file_spec, &device_spec);
		if (!converter)
		{
			MoonPrompt((char*)"[MoonMusicInit] 创建转换流失败");
			SDL_free(raw_data);
			return MOON_FALSE;
		}

		SDL_PutAudioStreamData(converter, raw_data, raw_len);

		int converted_len = SDL_GetAudioStreamAvailable(converter);
		if (converted_len <= 0)
		{
			MoonPrompt((char*)"[MoonMusicInit] 转换失败或无数据\n");
			SDL_DestroyAudioStream(converter);
			SDL_free(raw_data);
			return MOON_FALSE;
		}

		MoonAlloc((void**)&moon_music_sound, sizeof(MOON_CORE_MUSIC), moon_music_index + 1, "realloc");
		MoonAlloc((void**)&moon_music_sound[moon_music_index].data, converted_len, 1, "malloc");
		moon_music_sound[moon_music_index].length = converted_len / sizeof(float);
		SDL_GetAudioStreamData(converter, moon_music_sound[moon_music_index].data, converted_len);
		SDL_DestroyAudioStream(converter);

		MoonPrompt((char*)"[MoonMusicInit] 转换完成");

		music->id = moon_music_index;
		moon_music_index++;
	}
	
	SDL_free(raw_data);

	return MOON_TRUE;
}

_declspec(dllexport) extern int MoonSleep(int timeload)
{
	SDL_Delay(timeload);
	return timeload;
}

_declspec(dllexport) extern unsigned int MoonHash(char* text)
{
	if (text == MOON_NULL)
		return MOON_FALSE;
	unsigned int length = (unsigned int)strlen(text), hash = 0;
	if (length == 0)
		return MOON_FALSE;
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

_declspec(dllexport) extern unsigned char MoonKeyState(MOON_KEY_TYPE Key)
{
	if (moon_key[Key] == MOON_KEY_MODE_PRESS)
		return MOON_TRUE;
	else
		return MOON_FALSE;
}

_declspec(dllexport) extern unsigned char MoonKeyReal(MOON_KEY_TYPE Key)
{
	if (moon_key[Key] == MOON_KEY_MODE_PRESS_LONG)
		return MOON_TRUE;
	else
		return MOON_FALSE;
}

_declspec(dllexport) extern int MoonTimeLoad(MOON_TIMELOAD* Timeload, int mode)
{
	if (!mode)return MOON_FALSE;
	else
		if (!Timeload)
		{
			MoonPrompt((char*)"[TimeLoad函数错误!]存在空指针");
			return MOON_Error;
		}
	if (!Timeload->timeswitch)
	{
		Timeload->time1 = (unsigned int)SDL_GetTicks();
		if (Timeload->time1 > Timeload->time2 + Timeload->timeload)
		{
			Timeload->time2 = clock();
			Timeload->timeswitch = MOON_TRUE;
		}
	}
	else
	{
		Timeload->time2 = (unsigned int)SDL_GetTicks();
		Timeload->timeswitch = MOON_FALSE;
	}
	return Timeload->timeswitch;
}

_declspec(dllexport) extern void* MoonFindEntity(char* nameid)
{
	void* entity =	utility_core->entityindex[(MoonHash(nameid) % MOON_ENTITY_NUMBER)].entityindex;
	if (entity == MOON_NULL)
	{
		char text[255];
		snprintf(text, 255, "[MoonFindEntity]空指针错误!来自名称[%s]的实体", nameid);
		MoonProjectError(utility_core->entityindex[(MoonHash(nameid) % MOON_ENTITY_NUMBER)].entityindex, 1, text);
		return MOON_NULL;
	}
	return entity;
}

_declspec(dllexport) extern int MoonCreateEntityIndex(void* arrentity, char* nameid, size_t size_len, char* type_name)
{
	int index = MOON_NOTFOUND;
	int hash = MoonHash(nameid) % MOON_ENTITY_NUMBER;
	if (hash == MOON_FALSE)
		printf("非法的名称[%s],无法通过这个字符串得到合法的索引", nameid);

	index = hash;
	
	if (!utility_core->entityindex[hash].entityindex)
	{		
		utility_core->entityindex[index].entityindex = arrentity;
		utility_core->entityindex[index].nameid = nameid;
		utility_core->entityindex[index].length = (int)size_len;
		utility_core->entityindex[index].type_name = type_name;
	}
	else
	{
		printf("\n[CreateEntityIndex函数]报错\n试图注册为[%s]的实体\n在位置[%d]已有实体存在\n名称为[%s]\n请换一个名字\n", nameid, hash, utility_core->entityindex[index].nameid);
		if(strlen(nameid) <= 0) 
			MoonProjectError(&utility_core->entityindex[hash], 2, (char*)"来自[CreateEntityIndex函数]的错误,出现了幽灵实体,没有合法名称");
		index = MOON_NOTFOUND;
	}

	{
		char text[255];
		snprintf(text, 255, "创建了名称为[\033[31;46m%s\033[0m]的实体\n类型:%s\n地址:0x%p\nHash:%d\n索引:%d", nameid, utility_core->entityindex[hash].type_name, utility_core->entityindex[hash].entityindex, MoonHash(nameid), index);
		MoonPrompt(text);
	}

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
	button->mode = (MOON_BUTTON_TYPE)MOON_FALSE;
	button->triggermode = 1;
	return 1;
}

_declspec(dllexport) extern int MoonButtonDetection(MOON_BUTTON* button, int x, int y, void* context)
{
	static unsigned char mode = 0;
	if (
		button->x < x
		&& button->y < y
		&& button->x + button->width > x
		&& button->y + button->height > y
		)
	{
		button->mode = (MOON_BUTTON_TYPE)MoonKeyState(MOON_KEY_MOUSE_LEFT);
		if (button->mode == MOON_KEY_MODE_PRESS)
		{
			mode = MOON_TRUE;
			button->mode = MOON_BUTTON_PRESS;
			if (button->ButtonModePress)
				button->ButtonModePress(button, context);
			return MOON_BUTTON_PRESS;
		}
		else
			if (mode && button->mode == MOON_KEY_MODE_PRESS_LONG)
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
		if (button->ButtonModeFalse)
			button->ButtonModeFalse(button, context);
		return MOON_BUTTON_FALSE;
	}
}

_declspec(dllexport) extern int MoonButtonSetTriggerMode(MOON_BUTTON* button, unsigned int key)
{
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
		glfwSetInputMode(utility_core->hwnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
		
	case MOON_CURSOR_MODE_DISABLED:
		glfwSetInputMode(utility_core->hwnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
		
	case MOON_CURSOR_MODE_CAPTURED:
		glfwSetInputMode(utility_core->hwnd, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
		break;
	}
	return;
}

_declspec(dllexport) extern unsigned int MoonFileRead_Line(MOON_FILE* file, char* file_buffer, unsigned int line)
{
	if (!line || line > file->line_all)
	{
		MoonPrompt((char*)"[MoonFileRead_Line] 非法的行数");
		return MOON_FALSE;
	}
	unsigned int line_size = 0;

	{
		if (line == file->line_all)
			line_size = (unsigned int)strlen(file->file_buffer + file->line_index[line - 1]);
		else
			line_size = file->line_index[line] - file->line_index[line - 1];
	}

	char* buffer = (char*)calloc((line_size + 1), sizeof(char));
	if (!buffer)
	{
		MoonPrompt((char*)"[MoonFileRead_Line] 缓冲分配失败");
		return MOON_FALSE;
	}
	for (unsigned int index = 0; index < line_size; ++index)
		buffer[index] = file->file_buffer[file->line_index[line - 1] + index];
	strcpy(file_buffer, (const char*)buffer);
	free(buffer);
	return line_size;
}

_declspec(dllexport) extern unsigned char MoonFileRead_TEXT(MOON_FILE* file, const char* file_name)
{
	int file_size = 0;

	{
		if (!file)
		{
			MoonPrompt((char*)"[MoonFileRead_TEXT] 非法的[MOON_FILE]");
			return MOON_FALSE;
		}
		file->file_size = 0;
		file->line_all = 0;
	}

	{
		FILE* fp = fopen(file_name, "rb");

		if (!fp)
		{
			MoonProjectError(fp, 2, (char*)"[MoonFileRead_TEXT] 文件读取失败\n[MoonFileLoad] 为了防止进一步的失败,引擎将退出");
			MoonProjectDead();
			return MOON_FALSE;
		}

		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);

		if (file_size <= 0)
		{
			MoonProjectError(fp, 2, (char*)"[MoonFileRead_TEXT] 文件为空\n[MoonFileLoad] 为了防止进一步的失败,引擎将退出");
			MoonProjectDead();
			return MOON_FALSE;
		}

		fclose(fp);
	}

	{
		file->file_size = (unsigned int)file_size;

		if (!MoonAlloc((void**)&file->file_buffer, sizeof(char), file->file_size + 1, "malloc"))
			return MOON_FALSE;

		if (!MoonFileLoad_TEXT(file_name, (char*)file->file_buffer, file->file_size + 1))
		{
			if (!MoonFree(file->file_buffer))
			{
				MoonProjectError((void*)file->file_buffer, 3, (char*)"[MoonFileRead_TEXT] 找不到注册的字符串,内存无法释放");
				MoonProjectDead();
			}
			return MOON_FALSE;
		}

		MoonStrMatch_Replace((char*)file->file_buffer, 0, file->file_size, '\r', ' ');

		{
			file->line_all = 1;

			for (unsigned int index = 0; index < file->file_size; ++index)
				if (file->file_buffer[index] == '\n')
					++file->line_all;

			if (!MoonAlloc((void**)&file->line_index, sizeof(unsigned int), file->line_all + 1, "malloc"))
			{
				if (!MoonFree(file->file_buffer))
				{
					MoonProjectError((void*)file->file_buffer, 1, (char*)"[MoonFileRead_TEXT] 找不到注册的字符串,内存无法释放");
					MoonProjectDead();
				}
				return MOON_FALSE;
			}

			file->line_index[0] = 0;

			{
				int line_index = 1;
				for (unsigned int index = 0; index < file->file_size; ++index)
					if (file->file_buffer[index] == '\n')
					{
						file->line_index[line_index] = index + 1;
						line_index++;
					}
			}
			file->line_index[file->line_all] = file->file_size;
		}
	}
	return MOON_TRUE;
}

_declspec(dllexport) extern unsigned char MoonFileLoad_TEXT(const char* file_name, char* text, unsigned int text_size)
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

_declspec(dllexport) extern unsigned int MoonStrMatch_Prefix(const char* str_1, const char* str_2)
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

_declspec(dllexport) extern unsigned int MoonStrMatch_PrefixIgnore(const char* str_1, const char* str_2, char ch)
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

_declspec(dllexport) extern void MoonStrMatch_Replace(char* str, unsigned int start_index, unsigned int len, char ch_goal, char ch_replace)
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

_declspec(dllexport) extern unsigned int MoonStrMatch_PrefixIgnoreStr(const char* str_1, const char* str_2, const char* igno_str)
{
	unsigned int
		index_1 = 0,
		index_2 = 0,
		index_all = 0;
	if (!str_1 || !str_2)
	{
		MoonPrompt((char*)"[StrMatch_PrefixIgnoreStr]函数提示,str_1或者str_2参数传入不合理\n");
		return 0;
	}

	unsigned int igno_str_len = (unsigned int)strlen(igno_str);

	while (str_1[index_1] && str_2[index_2])
	{
		while (str_1[index_1])
		{
			int alpha = 0;
			for (unsigned int index = 0; index < igno_str_len; index++)
				if (str_1[index_1] == igno_str[index])
					alpha = 1;
			if (alpha)
				index_1++;
			else
				break;
		}

		while (str_2[index_2])
		{
			int alpha = 0;
			for (unsigned int index = 0; index < igno_str_len; index++)
				if (str_2[index_2] == igno_str[index])
					alpha = 1;

			if (alpha)
				index_2++;
			else
				break;
		}

		if ((!str_1[index_1] || !str_2[index_2])
			|| str_1[index_1] != str_2[index_2])
			break;
		index_all++;
		index_1++;
		index_2++;
	}
	return index_all;
}

static unsigned char MoonAlloc_Registry()
{
	void* alloc_buffer = realloc(moon_alloc.alloc, (size_t)(sizeof(MOON_ALLOC*) * (moon_alloc.index + 1)));
	if (alloc_buffer)
	{
		moon_alloc.alloc = (void**)alloc_buffer;
		moon_alloc.index += 1;
	}
	else
	{
		MoonPrompt((char*)"[MoonAlloc] 内存分配失败");
		return MOON_FALSE;
	}
	return MOON_TRUE;
}

_declspec(dllexport) extern unsigned char MoonAlloc(void** ptr, size_t size_len, unsigned int num, const char* alloc)
{
	void* ptr_buffer = MOON_NULL;
	if (strcmp(alloc, "realloc"))
	{
		for (unsigned int index = 0; index < moon_alloc.index; index++)
			if (*ptr == moon_alloc.alloc[index])
			{
				char text_buffer[255] = { 0 };
				snprintf(text_buffer, 255, "[MoonAlloc] 已存在 0x%p", ptr);
				MoonPrompt(text_buffer);
				if (strcmp(alloc, "malloc") || strcmp(alloc, "calloc"))
				{
					snprintf(text_buffer, 255, "[MoonAlloc] 无效的参数[%s]", alloc);
					MoonPrompt(text_buffer);
				}
				return MOON_FALSE;
			}

		if (!strcmp(alloc, "malloc"))
			ptr_buffer = malloc((size_t)(size_len * num));
		else
			if (!strcmp(alloc, "calloc"))
				ptr_buffer = calloc((size_t)num, size_len);
			else
			{
				char text_buffer[255] = { 0 };
				snprintf(text_buffer, 255, "[MoonAlloc] 无效的参数[%s]", alloc);
				MoonPrompt(text_buffer);
				return MOON_FALSE;
			}

		if (ptr_buffer)
		{
			if (MoonAlloc_Registry())
			{
				moon_alloc.alloc[moon_alloc.index - 1] = ptr_buffer;
				*ptr = ptr_buffer;
			}
		}
		else
		{
			MoonPrompt((char*)"[MoonAlloc] 内存分配失败");
			return MOON_FALSE;
		}
	}
	else
	{
		int re = 0, alloc_index = 0;
		for (unsigned int index = 0; index < moon_alloc.index; index++)
			if (*ptr == moon_alloc.alloc[index])
			{
				re = MOON_TRUE;
				alloc_index = index;
				break;
			}

		ptr_buffer = realloc(*ptr, (size_t)(size_len * num));
		if (ptr_buffer)
		{
			if (re || MoonAlloc_Registry())
			{
				if (!re)
					moon_alloc.alloc[moon_alloc.index - 1] = ptr_buffer;
				else
					moon_alloc.alloc[alloc_index] = ptr_buffer;
				*ptr = ptr_buffer;
			}
		}
		else
		{
			MoonPrompt((char*)"[MoonAlloc] 内存分配失败");
			return MOON_FALSE;
		}
	}
	return MOON_TRUE;
}

_declspec(dllexport) extern unsigned char MoonFree(void* ptr)
{
	for (unsigned int index = 0; index < moon_alloc.index; index++)
		if (moon_alloc.alloc[index] == ptr)
		{
			free(ptr);
			ptr = MOON_NULL;
			moon_alloc.alloc[index] = MOON_NULL;
			return MOON_TRUE;
		}
	return MOON_FALSE;
}

_declspec(dllexport) extern inline float MoonDegRad(float phi)
{
	return (MOON_Pi * (phi) * 1.f / 180.f);
}

_declspec(dllexport) extern void MoonShaderUniform(unsigned int shader, const char* var, MOON_UNIFORM_DATA* data)
{
	if (strlen(var) > MOON_MESSAGE_TEXT_MAX - 1)
	{
		MoonPrompt((char*)"[MoonShaderUniform]函数 变量名称超出[MOON_MESSAGE_TEXT_MAX],请尝试缩短变量名");
		return;
	}
	MOON_METADATA metadata = { 0 };
	//strcpy((char*)metadata.uniform.var, var);
	for (unsigned int index = 0; index < MOON_MESSAGE_TEXT_MAX - 1; index++)
		metadata.uniform.var[index] = var[index];
	metadata.uniform.shader = shader;
	metadata.uniform.data = *data;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_UNIFORM, metadata);
}

_declspec(dllexport) extern unsigned char MoonMatrix4_4Mul(float* mat4_return, float* mat4_left, float* mat4_right)
{
	if (!mat4_return || !mat4_left || !mat4_right)
	{
		MoonPrompt((char*)"[MoonMatrix4_4Mul] 空指针");
		return MOON_FALSE;
	}

	float mat4_return_buffer[16];

	for (int col = 0; col < 4; ++col)
		for (int row = 0; row < 4; ++row)
		{
			float num = 0;
			for (int dim = 0; dim < 4; ++dim)
				num += mat4_left[dim * 4 + row] * mat4_right[col * 4 + dim];
			mat4_return_buffer[col * 4 + row] = num;
		}

	for (int index = 0; index < 16; index++)
		mat4_return[index] = mat4_return_buffer[index];

	return MOON_TRUE;
}

_declspec(dllexport) extern int MoonGetFps()
{
	return *fps;
}

_declspec(dllexport) extern inline int MoonRandom(unsigned int seed, int start, int end)
{
	if (start >= end)
		return start;
	static unsigned int normal_seed, old_seed;
	if (seed && old_seed != seed)
	{
		old_seed = seed;
		normal_seed = seed;
	}
	if (normal_seed == 0)
		normal_seed = 123456789u;
	normal_seed = (normal_seed * 1103515245 + 12345) & 0x7fffffff;
	int rand_out = ((normal_seed >> 16) | ((normal_seed & 0xffff) << 16)) % (end - start) + start;
	return rand_out;
}

//初始化
_declspec(dllexport) extern unsigned char MoonVectorInit(MOON_VECTOR* vector, float* num, unsigned int num_size)
{
	float* buffer = (float*)realloc(vector->vector, (size_t)(sizeof(float) * num_size));
	if (!buffer)
		return MOON_FALSE;
	vector->vector = buffer;
	for (unsigned int index = 0; index < num_size; ++index)
		vector->vector[index] = num[index];
	vector->dim = num_size;
	return MOON_TRUE;
}

//初始化
_declspec(dllexport) extern void MoonVectorFree(MOON_VECTOR* vector)
{
	if (!vector)
		return;
	vector->dim = 0;
	free(vector->vector);
	vector->vector = (float*)MOON_NULL;
}

//返回维度
_declspec(dllexport) extern unsigned int MoonVector_Dim(MOON_VECTOR* vector)
{
	if (!vector)
		return 0;
	return vector->dim;
}

//获取元素
_declspec(dllexport) extern float MoonVector_Get(MOON_VECTOR* vector, unsigned int dim)
{
	//在高维度下, 值为0
	if (!vector || !dim || dim > vector->dim)
		return 0.f;
	else
		return vector->vector[dim - 1];
}

//扩容维度
_declspec(dllexport) extern unsigned char MoonVector_SetDim(MOON_VECTOR* vector, unsigned int dim)
{
	if (!vector || !dim)
		return MOON_FALSE;
	if (dim > vector->dim)
	{
		float* buffer = (float*)realloc(vector->vector, (size_t)(sizeof(float) * dim));
		if (!buffer)
			return MOON_FALSE;
		vector->vector = buffer;
		memset(vector->vector + vector->dim, 0, (dim - vector->dim) * sizeof(float));
		vector->dim = dim;
	}
	return MOON_TRUE;
}

//设置元素
_declspec(dllexport) extern unsigned char MoonVector_SetEle(MOON_VECTOR* vector, unsigned int dim, float num)
{
	if (!MoonVector_SetDim(vector, dim))
		return MOON_FALSE;
	vector->vector[dim - 1] = num;
	return MOON_TRUE;
}

//获取模长
_declspec(dllexport) extern float MoonVector_Norm(MOON_VECTOR* vector)
{
	if (!vector || !vector->dim)
		return -1.f;
	float
		norm = 1.f,
		scale = 0.0f;
	for (unsigned int index = 0; index < vector->dim; ++index)
		if (vector->vector[index] != 0.f)
		{
			float num = fabsf(vector->vector[index]);
			if (scale < num)
			{
				scale = num;
				norm = 1.f + norm * (scale / num) * (scale / num);
			}
			else
				norm = norm + (num / scale) * (num / scale);
		}
	return scale * sqrtf(norm);
}

//归一化为单位向量
_declspec(dllexport) extern void MoonVector_NormSize(MOON_VECTOR* vector_out, MOON_VECTOR* vector)
{
	if (!vector || !vector_out || !vector->dim)
	{
		if (!vector)
			MoonPrompt((char*)"[MoonVector_NormSize]函数 vector空指针错误");
		else
			if (!vector_out)
				MoonPrompt((char*)"[MoonVector_NormSize]函数 vector_out空指针错误");
		return;
	}

	{
		float norm = MoonVector_Norm(vector);
		if (norm <= 0.f)
		{
			MoonPrompt((char*)"[MoonVector_NormSize]函数 Norm异常");
			return;
		}
		float* vector_buffer = (float*)malloc(vector->dim * sizeof(float));
		if (!vector_buffer)
		{
			MoonPrompt((char*)"[MoonVector_NormSize]函数 内存分配失败");
			return;
		}
		for (unsigned int index = 0; index < vector->dim; ++index)
			vector_buffer[index] = vector->vector[index] / norm;
		free(vector_out->vector);
		vector_out->vector = vector_buffer;
		vector_out->dim = vector->dim;
	}
}

//数乘
_declspec(dllexport) extern void MoonVector_Scale(MOON_VECTOR* vector_out, MOON_VECTOR* vector, float num)
{
	if (!vector || !vector_out || !vector->dim)
	{
		if (!vector)
			MoonPrompt((char*)"[MoonVector_Scale]函数 vector空指针错误");
		else
			if (!vector_out)
				MoonPrompt((char*)"[MoonVector_Scale]函数 vector_out空指针错误");
		return;
	}

	{
		if (vector->dim != vector_out->dim)
		{
			float* vector_buffer = (float*)malloc(vector->dim * sizeof(float));
			if (!vector_buffer)
			{
				MoonPrompt((char*)"[MoonVector_Scale]函数 内存分配失败");
				return;
			}
			for (unsigned int index = 0; index < vector->dim; ++index)
				vector_buffer[index] = vector->vector[index] * num;
			free(vector_out->vector);
			vector_out->vector = vector_buffer;
		}
		else
		{
			for (unsigned int index = 0; index < vector->dim; ++index)
				vector_out->vector[index] = vector->vector[index] * num;
		}
	}

	vector_out->dim = vector->dim;
}

//向量加法
_declspec(dllexport) extern void MoonVector_Add(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2)
{
	if (!vector_1 || !vector_2 || !vector_out)
	{
		if (!vector_1)
			MoonPrompt((char*)"[MoonVector_Add]函数 vector_1空指针错误");
		else
			if (!vector_2)
				MoonPrompt((char*)"[MoonVector_Add]函数 vector_2空指针错误");
			else
				MoonPrompt((char*)"[MoonVector_Add]函数 vector_out空指针错误");
		return;
	}

	{
		unsigned int
			dim_max = MoonMax(vector_1->dim, vector_2->dim),
			dim_min = MoonMin(vector_1->dim, vector_2->dim);
		if (vector_out->dim != dim_max)
			if (!MoonVector_SetDim(vector_out, dim_max))
				return;
		for (unsigned int index = 0; index < dim_min; ++index)
			vector_out->vector[index] = vector_1->vector[index] + vector_2->vector[index];

		if (dim_max == dim_min)
			return;

		if (vector_1->dim > vector_2->dim)
			for (unsigned int index = dim_min; index < dim_max; ++index)
				vector_out->vector[index] = vector_1->vector[index];
		else
			for (unsigned int index = dim_min; index < dim_max; ++index)
				vector_out->vector[index] = vector_2->vector[index];
		vector_out->dim = dim_max;
	}
}

//向量减法
_declspec(dllexport) extern void MoonVector_Sub(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2)
{
	if (!vector_1 || !vector_2 || !vector_out)
	{
		if (!vector_1)
			MoonPrompt((char*)"[MoonVector_Sub]函数 vector_1空指针错误");
		else
			if (!vector_2)
				MoonPrompt((char*)"[MoonVector_Sub]函数 vector_2空指针错误");
			else
				MoonPrompt((char*)"[MoonVector_Sub]函数 vector_out空指针错误");
		return;
	}

	{
		unsigned int
			dim_max = MoonMax(vector_1->dim, vector_2->dim),
			dim_min = MoonMin(vector_1->dim, vector_2->dim);
		if (vector_out->dim != dim_max)
			if (!MoonVector_SetDim(vector_out, dim_max))
				return;
		for (unsigned int index = 0; index < dim_min; ++index)
			vector_out->vector[index] = vector_1->vector[index] - vector_2->vector[index];

		if (dim_max == dim_min)
			return;

		if(vector_1->dim > vector_2->dim)
			for (unsigned int index = dim_min; index < dim_max; ++index)
				vector_out->vector[index] = vector_1->vector[index];
		else
			for (unsigned int index = dim_min; index < dim_max; ++index)
				vector_out->vector[index] = -vector_2->vector[index];
		vector_out->dim = dim_max;
	}
}

//哈达玛积
_declspec(dllexport) extern void MoonVector_Hadamard(MOON_VECTOR* vector_out, MOON_VECTOR* vector_1, MOON_VECTOR* vector_2)
{
	if (!vector_1 || !vector_2 || !vector_out)
	{
		if (!vector_1)
			MoonPrompt((char*)"[MoonVector_Hadamard]函数 vector_1空指针错误");
		else
			if (!vector_2)
				MoonPrompt((char*)"[MoonVector_Hadamard]函数 vector_2空指针错误");
			else
				MoonPrompt((char*)"[MoonVector_Hadamard]函数 vector_out空指针错误");
		return;
	}

	{
		unsigned int
			dim_max = MoonMax(vector_1->dim, vector_2->dim),
			dim_min = MoonMin(vector_1->dim, vector_2->dim);
		if (vector_out->dim != dim_max)
			if (!MoonVector_SetDim(vector_out, dim_max))
				return;
		for (unsigned int index = 0; index < dim_min; ++index)
			vector_out->vector[index] = vector_1->vector[index] * vector_2->vector[index];
		if (dim_max > dim_min)
			memset(vector_out->vector + dim_min, 0, (dim_max - dim_min) * sizeof(float));
		vector_out->dim = dim_max;
	}
}

//点积
_declspec(dllexport) extern float MoonVector_Dot(MOON_VECTOR* vector_1, MOON_VECTOR* vector_2)
{
	if (!vector_1 || !vector_2)
	{
		//因为无法分清合法与非法,故报错
		if (!vector_1)
			MoonProjectError(vector_1, 2, (char*)"[MoonVector_Dot]函数 vector_1为空指针");
		else
			MoonProjectError(vector_2, 2, (char*)"[MoonVector_Dot]函数 vector_2为空指针");
		return 0.f;
	}
	float dot = 0;
	if (!vector_1->dim || !vector_2->dim)
		return 0.f;
	{
		unsigned int dim_min = MoonMin(vector_1->dim, vector_2->dim);
		for (unsigned int index = 0; index < dim_min; ++index)
			 dot += vector_1->vector[index] * vector_2->vector[index];
	}
	return dot;
}

_declspec(dllexport) extern MOON_POINT2D MoonCursorOffect(MOON_POINT2D size)
{
	MOON_POINT2D game_coord;
	static MOON_POINT2D last_mouse = { 0 };
	static MOON_POINT2D last_game = { 0 };
	MOON_POINT2D offect;
	offect.x = moon_mouse_coord->x - last_mouse.x;
	offect.y = moon_mouse_coord->y - last_mouse.y;
	MOON_POINT2D new_game_coord; new_game_coord.x = last_game.x + offect.x; 
	new_game_coord.y = last_game.y + offect.y;
	new_game_coord.x = MoonRange(new_game_coord.x, 0, size.w);
	new_game_coord.y = MoonRange(new_game_coord.y, 0, size.h);
	game_coord = new_game_coord;
	last_mouse = *moon_mouse_coord;
	last_game = new_game_coord;
	return game_coord;
}

_declspec(dllexport) extern MOON_POINT2D MoonCursorGet()
{
	return *moon_mouse_coord;
}

_declspec(dllexport) extern void MoonSetPower(unsigned char power)
{
	MOON_METADATA metadata = { 0 };
	metadata.power = power;
	MoonProjectSendMessage(MOON_MESSAGE_POWER, metadata);
}

_declspec(dllexport) extern inline MOON_IMAGE* MoonProjectBuffer()
{
	return moon_engineback;
}


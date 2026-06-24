#include"Moon.h"
#include"MoonCore.h"

static MOON_ENGINECORE* utility_project;
static MOON_ALLOC_REGISTRY moon_alloc;

static MOON_CORE_MUSIC* moon_music_sound;
static unsigned int moon_music_index;
static SDL_AudioDeviceID moon_audio_dev = 0;
static SDL_AudioStream* moon_audio_stream = (SDL_AudioStream*)MOON_NULL;
static int* fps;

/*
* 函數 MoonAlloc_Registry
* 作用 内部注册表名单+1
* 使用方法
* MoonAlloc_Registry();
*/
static _Bool MoonAlloc_Registry();

extern void MoonUtilityLoad(MOON_ENGINECORE* project)
{
	utility_project = project;
	{
		SDL_AudioSpec audio_spec =
		{
			.format = SDL_AUDIO_F32LE,
			.channels = 1,
			.freq = MOON_SAMPLE_RATE,
		};

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

		MoonHashFindEntity(project, (char*)"ProjectFPS", int, fpsnumber);
		fps = fpsnumber;
	}

}

extern void MoonUtilityOver()
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

extern inline void MoonMusicSet(MOON_MUSIC* music,MOON_MUSIC_MODE on_or_off,float start,float end)
{
	music->mode = on_or_off;
	music->start = start;
	music->end = end;
}

extern inline void MoonMusicAgain(MOON_MUSIC* music)
{
	music->mode = MOON_MUSIC_MODE_AGAIN;
}

extern int MoonMusic(MOON_MUSIC* music)
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

extern _Bool MoonMusicInit_Wav(MOON_MUSIC* music, const char* File)
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

	SDL_AudioSpec file_spec = { 0 };

	if (!SDL_LoadWAV(File, &file_spec, &raw_data, &raw_len)) 
	{
		MoonPrompt((char*)"");
		printf("[MoonMusicInit] 加载失败: %s\n", SDL_GetError());
		return MOON_FALSE;
	}

	{
		SDL_AudioSpec device_spec =
		{
			.format = SDL_AUDIO_F32LE,
			.channels = 1,
			.freq = MOON_SAMPLE_RATE,
		};

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

extern int MoonSleep(int timeload)
{
	SDL_Delay(timeload);
	return timeload;
}

extern unsigned int MoonHash(char* text)
{
	if (text == MOON_NULL)
		return MOON_FALSE;
	unsigned int length = (unsigned int)strlen(text), hash = 0;
	if (length == 0)
		return MOON_FALSE;
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
	static unsigned char KEYSTATEbuffer[MOON_KEY_LAST];
	int state = glfwGetKey(utility_project->hwnd, (int)Key) || glfwGetMouseButton(utility_project->hwnd, (int)Key);
	if (state == GLFW_RELEASE)
		KEYSTATEbuffer[Key] = 0;
	else 
		if (state == GLFW_PRESS && KEYSTATEbuffer[Key] == 0)
	{ 
		KEYSTATEbuffer[Key] = MOON_TRUE; 
		return MOON_TRUE; 
	}
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
	void* entity = project->entityindex[(MoonHash(nameid) % MOON_ENTITY_NUMBER)].entityindex;
	if (entity == MOON_NULL)
	{
		char text[255];
		snprintf(text, 255, "[MoonFindEntity]空指针错误!来自名称[%s]的实体", nameid);
		MoonProjectError(project->entityindex[(MoonHash(nameid) % MOON_ENTITY_NUMBER)].entityindex, 1, text);
		return MOON_NULL;
	}
	return entity;
}

extern int MoonCreateEntityIndex(MOON_PROJECTGOD* project, void* arrentity, char* nameid, size_t size_len, char* type_name)
{
	int index = MOON_NOTFOUND;
	int hash = MoonHash(nameid) % MOON_ENTITY_NUMBER;
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
	button->mode = (MOON_BUTTON_TYPE)MOON_FALSE;
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
		{
			MOON_METADATA metadata = { 0 };
			metadata.key.token = button->triggermode;
			metadata.key.worth = (int*)&(button->mode);
			MoonProjectSendMessage(MOON_MESSAGE_KEY, metadata);
		}
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

extern unsigned int MoonStrMatch_PrefixIgnoreStr(const char* str_1, const char* str_2, const char* igno_str)
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

static _Bool MoonAlloc_Registry()
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

extern _Bool MoonAlloc(void** ptr, size_t size_len, unsigned int num, const char* alloc)
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

extern _Bool MoonFree(void* ptr)
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

extern inline float MoonDegRad(float phi)
{
	return (MOON_Pi * (phi) * 1.f / 180.f);
}

extern void MoonShaderUniform(unsigned int shader, const char* var, MOON_UNIFORM_DATA* data)
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

extern _Bool MoonMatrix4_4Mul(float* mat4_return, float* mat4_left, float* mat4_right)
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

extern int MoonGetFps()
{
	return *fps;
}
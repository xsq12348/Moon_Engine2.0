#include"MoonCore.h"

static unsigned char Moon_Engine_VSn[4] = { 2,1,0,9 };
static MOON_TIMELOAD projectfps;
static int fpsmax, fpsmax2;
static MOON_IMAGE projectdoublebuffer;
static MOON_POINT2D projectmousecoord;
static MOON_ENTITYINDEX entityindex[ENTITYNUMBER];
static MOON_ENGINECORE moon_engine_core;
static _Bool thread_draw_type, thread_attr_type;

static const char* moon_vertex_shader2d_code =
"#version 330 core\n"
MoonString(
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
out vec4 vcolor;
void main()
{
	vcolor = color;
	gl_Position = vec4(position, 1.0);
}
);

static const char* moon_pixel_shader2d_code =
"#version 330 core\n"
MoonString(
	out vec4 pixel_color;
in vec4 vcolor;
void main()
{
	pixel_color = vec4(vcolor);
}
);

static const char* moon_vertex_shader2d_texture_code =
"#version 330 core\n"
MoonString(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 atex_uv;
out vec2 tex_uv;
void main()
{
	tex_uv = atex_uv;
	gl_Position = vec4(position, 1.0);
}
);

static const char* moon_pixel_shader2d_texture_code =
"#version 330 core\n"
MoonString(
in vec2 tex_uv;
out vec4 pixel_color;
uniform sampler2D moon_utexture;
void main()
{
	pixel_color = texture(moon_utexture, tex_uv);
}
);

static GLuint shader_program_vectex, shader_program_pixel;

static _Bool ProjectConsole(MOON_PROJECTGOD* project, int (*developerconsole)(MOON_PROJECTGOD*));	//控制台
static MOON_PROJECTMODULE(MoonLogicPause);															//暂停逻辑线程
static MOON_PROJECTMODULE(MoonDrawingPause);														//暂停绘制线程
static CREATETHREADFUNCTION(ProjectLogicThread);													//逻辑线程

extern MOON_HWND* MoonWindow(const char* name, int window_coord_x, int window_coord_y, int window_width, int window_height)
{
	MOON_HWND* hwnd;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
	hwnd = glfwCreateWindow(window_width, window_height, name, MOON_NULL, MOON_NULL);
	glfwSetWindowPos(hwnd, window_coord_x, window_coord_y);
	glfwMakeContextCurrent(hwnd);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		MoonProjectError(glfwGetProcAddress, 1, "[MoonWindow]严重错误,glfwGetProcAddress初始化失败");
		return 0;
	}
	//glfwShowWindow(hwnd);
	printf("OpenGL: %s\n", glGetString(GL_VERSION));
	unsigned int vao;
	glad_glGenVertexArrays(1, &vao);
	glad_glBindVertexArray(vao);
	glad_glClearColor(0.f, 0.f, 0.f, 1.f);
	glad_glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(hwnd);
	return hwnd;
}

extern void MoonProjectInit(MOON_PROJECTGOD* project, const char* project_name, int x, int y, int width, int height, int fps, void (*ProjectSetting_1)(MOON_PROJECTGOD*))
{
	MoonPrompt("[ProjectInit]初始化函数进入成功");
	printf("\n\033[1;33m    ████████████      \n  ████      ████████  \n  ██    ██████    ██  \n████  ██    ████    ███\n██████████████  ██  ███\n██  ██  ██  ██  ██  ███\n██  ██  ███████████████\n██    ████    ██  █████\n  ██    ██████    ██  \n  ████████      ████  \n      ████████████      \n\033[0m\n");
	printf("MoonEngine[%d.%d.%d.%d]\n", Moon_Engine_VSn[0], Moon_Engine_VSn[1], Moon_Engine_VSn[2], Moon_Engine_VSn[3]);
	/*
	  ██████████
  ████      ████████
  ██    ██████    ██
████  ██    ████    ██
██████████████  ██  ██
██  ██  ██  ██  ██  ██
██  ██  ██████████████
██    ████    ██  ████
  ██    ██████    ██
  ████████      ████
	  ██████████

	*/
	
	if (!glfwInit())
	{
		MoonProjectError(glfwInit, 1, "[MoonWindow]严重错误,glfwInit初始化失败");
		moon_engine_core.dead = MOON_TRUE;
		return;
	}

	GLFWmonitor* moon_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* moon_screen = glfwGetVideoMode(moon_monitor);

	if (x < 0 || y < 0)
	{
		x = (moon_screen->width - width) / 2;
		y = (moon_screen->height - height) / 2;
	}

	project->hwnd = MoonWindow(project_name, x, y, width, height);
	if (project->hwnd == MOON_NULL)
	{
		MoonProjectError(project->hwnd, 1, "[MoonWindow]严重错误,窗口创建失败");
		moon_engine_core.dead = MOON_TRUE;
		return;
	}

	//SDL_Init(SDL_INIT_AUDIO);

	MoonPrompt("projectgod初始化成功");
	project->project_name = project_name;
	project->window_height = height;
	project->window_width = width;
	moon_engine_core.dead = MOON_FALSE;
	project->entityindex = entityindex;
	MoonTimeLoadInit(&moon_engine_core.timeload, 1000.f / (fps > 0 ? fps : 60));
	MoonTimeLoadInit(&projectfps, 1000);
	MoonCreateEntityIndex(project, &fpsmax2, (char*)"ProjectFPS", sizeof(int), (char*)"int");
	MoonCreateEntityIndex(project, &projectmousecoord, (char*)"ProjectMouseCoord", sizeof(MOON_POINT2D), (char*)"MOON_POINT2D");
	MoonCreateEntityIndex(project, &projectdoublebuffer, (char*)"ProjectBitmap", sizeof(MOON_IMAGE), (char*)"MOON_IMAGE");

	MoonImageCreate(project, &projectdoublebuffer, project->window_width, project->window_height);

	MoonCreateEntityIndex(project, &shader_program_vectex, (char*)"ProjectShader_SolidColor", sizeof(unsigned int), (char*)"unsigned int");
	MoonCreateEntityIndex(project, &shader_program_pixel, (char*)"ProjectShader_Texture", sizeof(unsigned int), (char*)"unsigned int");
	MoonShaderLoad(&moon_vertex_shader2d_code, &moon_pixel_shader2d_code, &shader_program_vectex);				//加载渲染器
	MoonShaderLoad(&moon_vertex_shader2d_texture_code, &moon_pixel_shader2d_texture_code, &shader_program_pixel);	//加载渲染器
	MoonUtilityLoad(project);
	MoonDrawLoad(project);

	if (ProjectSetting_1 != MOON_NULL)ProjectSetting_1(project);
	MoonPrompt("[ProjectInit]初始化完成");

	glfwMakeContextCurrent(MOON_NULL);
}

static MOON_CREATETHREADFUNCTION(ProjectLogicThread)
{
	//等待绘制线程
	while (!moon_engine_core.thread_message_type_draw);
	//逻辑线程
	MOON_GETTHREADRESOURCE(MOON_PROJECTGOD*, project);

	MoonPrompt("加载了逻辑线程");

	while (!moon_engine_core.dead)
	{
		moon_engine_core.Logic(project);

		//自动锁
		MoonProjectGetMessage(project, &moon_engine_core.message_logic, &moon_engine_core.thread_message_type_logic, MoonlogicMessageHandle);
	}
	return 1;
}

static MOON_CREATETHREADFUNCTION(ProjectDrawingThread)
{
	//等待属性线程
	while (!moon_engine_core.thread_message_type_attr);
	//绘制线程
	//OpenGL
	MOON_GETTHREADRESOURCE(MOON_PROJECTGOD*, project);
	glfwMakeContextCurrent(project->hwnd);

	MoonUtilityLoad(project);	//重新绑定

	MoonHashFindEntity(project, "ProjectBitmap", MOON_IMAGE, projectbitmap);
	MoonHashFindEntity(project, "ProjectShader_SolidColor", GLuint, shader_program_1);
	MoonHashFindEntity(project, "ProjectShader_Texture", GLuint, shader_program_2);
	GLuint moon_vbo, moon_ebo, moon_vao, solid_color_shader, texture_shader;
	solid_color_shader = (*shader_program_1);
	texture_shader = (*shader_program_2);
	float vertexs[20] =
	{
		 1.f,  1.f, 0.f,  1.f, 1.f,  // 右上
		 1.f, -1.f, 0.f,  1.f, 0.f,  // 右下
		-1.f, -1.f, 0.f,  0.f, 0.f,  // 左下
		-1.f,  1.f, 0.f,  0.f, 1.f,  // 左上
	};
	unsigned int vertex_index[6] = { 0,1,2,2,3,0 };
	int runload[3] = { 0 };//帧率计时器

	{
		glad_glGenBuffers(1, &moon_vbo);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);

		glad_glGenBuffers(1, &moon_ebo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
		glad_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_index), vertex_index, GL_STATIC_DRAW);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glad_glGenVertexArrays(1, &moon_vao);
		glad_glBindVertexArray(moon_vao);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);

		glad_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glEnable(GL_BLEND);
		glad_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	MoonPrompt("加载了绘制线程");

	glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);

	//主循环
	while (!moon_engine_core.dead)
	{
		runload[0] = clock();
		{
			if (!MoonTimeLoad(&projectfps, MOON_TRUE))fpsmax++;
			else { fpsmax2 = fpsmax; fpsmax = 0; }
		}


		{
			moon_engine_core.Drawing(project);

			//自动锁
			MoonProjectGetMessage(project, &moon_engine_core.message_draw, &moon_engine_core.thread_message_type_draw, MoonDrawMessageHandle);

			glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glad_glViewport(0, 0, projectbitmap->image_size.w, projectbitmap->image_size.h);
			glad_glBindTexture(GL_TEXTURE_2D, projectbitmap->image.texture);
			MoonImageShader(texture_shader);
			glad_glUniform1i(glGetUniformLocation(texture_shader, "moon_utexture"), 0);
			glad_glBindVertexArray(moon_vao);
			glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo);
			glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moon_ebo);
			glad_glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glfwSwapBuffers(project->hwnd);
			MoonImageDesignated(projectbitmap);
			glad_glClearColor(0.f, 0.f, 0.f, 1.f);
			glad_glClear(GL_COLOR_BUFFER_BIT);
		}

		runload[1] = clock();
		runload[2] = runload[1] - runload[0];
		if (moon_engine_core.power <= 0)
			if (runload[2] < moon_engine_core.timeload.timeload)
				MoonSleep((moon_engine_core.timeload.timeload - runload[2]));
	}

	{
		glad_glDeleteVertexArrays(1, &moon_vao);
		glad_glDeleteBuffers(1, &moon_vbo);
		glad_glDeleteBuffers(1, &moon_ebo);
		glfwMakeContextCurrent(MOON_NULL);
	}

	thread_draw_type = MOON_TRUE;

	return 1;
}

extern void MoonProjectRun(MOON_PROJECTGOD* project, void (*ProjectSetting_2)(MOON_PROJECTGOD*), int(*ProjectLogic)(MOON_PROJECTGOD*), int(*ProjectDrawing)(MOON_PROJECTGOD*))
{
	MoonPrompt("[ProjectRun]引擎流程函数进入成功!");
	
	//默认在窗口内部
	glfwSetInputMode(project->hwnd, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
	
	if (ProjectDrawing == MOON_NULL)
	{
		MoonProjectError(ProjectDrawing, 1, (char*)"绘图函数传入失败!");
		return;
	}
	moon_engine_core.Drawing = ProjectDrawing;

	if (ProjectSetting_2 != MOON_NULL)ProjectSetting_2(project);
	MOON_CREATETHREAD(ProjectDrawingThread, "DrawingThread", project);//加载属性线程

	//加载逻辑线程
	if (ProjectLogic != MOON_NULL)
	{
		moon_engine_core.Logic = ProjectLogic;
		MOON_CREATETHREAD(ProjectLogicThread, "LogicThread", project);
	}
	else
		moon_engine_core.thread_message_type_logic = MOON_TRUE;

	MoonPrompt("加载了属性线程");

	MoonHashFindEntity(project, (char*)"ProjectMouseCoord", MOON_POINT2D, mousecoord);
	MoonHashFindEntity(project, "ProjectBitmap", MOON_IMAGE, projectbitmap);
	MoonHashFindEntity(project, (char*)"ProjectFPS", int, fpsnumber);
	static int(*drawing)(MOON_PROJECTGOD*) = 0, (*logic)(MOON_PROJECTGOD*) = 0, developer = MOON_FALSE;
	drawing = moon_engine_core.Drawing;
	logic = moon_engine_core.Logic;
	static double mousecoord_x_2, mousecoord_y_2;
	moon_engine_core.gamepowermode = moon_engine_core.power;
	int modetemp = MOON_FALSE, runload[3] = { 0 };//帧率计时器
	//属性线程
	while (!moon_engine_core.dead)
	{
		moon_engine_core.Logic != MoonLogicPause && (logic = moon_engine_core.Logic);
		moon_engine_core.Drawing != MoonDrawingPause && (drawing = moon_engine_core.Drawing);
		moon_engine_core.dead = (_Bool)glfwWindowShouldClose(project->hwnd);
		moon_engine_core.focus = (_Bool)!glfwGetWindowAttrib(project->hwnd, GLFW_FOCUSED);
		glfwPollEvents();

		{
			glfwGetCursorPos(project->hwnd, &mousecoord_x_2, &mousecoord_y_2);
			mousecoord->x = (int)mousecoord_x_2;
			mousecoord->y = (int)mousecoord_y_2;

			if (developer || moon_engine_core.focus)
				moon_engine_core.power = MOON_NOTFOUND;
			else
				moon_engine_core.power = moon_engine_core.gamepowermode;

			//当Power改变时
			moon_engine_core.power != modetemp && MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Logic, MoonLogicPause, logic);
			moon_engine_core.power != modetemp && MoonProjectPause(moon_engine_core.power < 0, &moon_engine_core.Drawing, MoonDrawingPause, drawing);
			modetemp = moon_engine_core.power;
			!developer && MoonKeyState(MOON_KEY_OEM_3) && (developer = MOON_TRUE);
			developer && (ProjectConsole(project, project->developerconsole), (MoonKeyState(MOON_KEY_OEM_3) && (developer = MOON_FALSE)));

			//自动锁
			MoonProjectGetMessage(project, &moon_engine_core.message_attr, &moon_engine_core.thread_message_type_attr, MoonAttrMessageHandle);
		}
		MoonSleep(1);
	}
	thread_attr_type = MOON_TRUE;
}

extern void MoonProjectOver(MOON_PROJECTGOD* project, void (*ProjectOverSetting)(MOON_PROJECTGOD*))
{
	while (1)
		if (thread_attr_type && thread_draw_type)
		{
			glfwMakeContextCurrent(project->hwnd);
			break;
		}
		else
			MoonSleep(1);

	MoonPrompt("[ProjectOver]引擎流程结束!\n[ProjectOver]结束函数进入成功");
	if (project == MOON_NULL)
	{
		MoonProjectError(project, 2, (char*)"核心对象[projectgod]丢失!");
		return;
	}
	if (ProjectOverSetting != 0)ProjectOverSetting(project);
	MoonDrawOver();
	//SDL_Quit();
	glad_glDeleteProgram(shader_program_vectex);
	glad_glDeleteProgram(shader_program_pixel);
	glfwTerminate();

	//釋放所有實體
	for (int index = 0; index < ENTITYNUMBER; index++)
	{
		if (project->entityindex[index].type_name != 0)
			if (!strcmp(project->entityindex[index].type_name, (char*)"MOON_IMAGE"))MoonImageDelete((MOON_IMAGE*)project->entityindex[index].entityindex);
			else if (!strcmp(project->entityindex[index].type_name, (char*)"MOON_ANIME"))MoonAnimeDelete((MOON_ANIME*)project->entityindex[index].entityindex);
		project->entityindex[index].length = MOON_NULL;
		project->entityindex[index].nameid = MOON_NULL;
		project->entityindex[index].entityindex = MOON_NULL;
	}
	MoonPrompt("[ProjectOver]资源清理完成");
	MoonPrompt("程序已退出");
}

extern int MoonProjectError(void* alpha, int degree, char* text)
{
	enum
	{
		Serious = 1,
		General,
		Mild,
	};
	printf("\n来自[%p]的[%s]发生错误!现在转入错误处理函数[ProjectError]", alpha, text);
	switch (degree)
	{
	case Serious:printf("\t等级[Serious/严重]\n"); break;
	case General:printf("\t等级[General/一般]\n"); break;
	case Mild:printf("\t等级[Mild/轻微]\n"); break;
	}
	while (!MoonKeyState(MOON_KEY_ESCAPE)) MoonSleep(1);
	return degree;
}

extern int MoonProjectPause(int mode, int (**function_1)(MOON_PROJECTGOD*), int (*function_2)(MOON_PROJECTGOD*), int (*function_3)(MOON_PROJECTGOD*))
{
	if (mode) *function_1 = function_2;
	else *function_1 = function_3;
	return 1;
}

extern void MoonProjectFunctionSwitch(char module, int (*function_2)(MOON_PROJECTGOD*))
{
	if (function_2 == MOON_NULL)
	{
		char text[255] = { MOON_NULL };
		snprintf(text, 255, "[MoonProjectFunctionSwitch]函数,空指针错误,请勿传入空指针,来自模块[%s]", (module == MOON_MODULE_DRAW ? "Draw" : (module == MOON_MODULE_ATTR ? "Attr" : "Logic")));
		MoonPrompt(text);
	}
	switch (module)
	{
	case MOON_MODULE_DRAW:	moon_engine_core.Drawing = function_2; break;
	case MOON_MODULE_LOGIC:	moon_engine_core.Logic = function_2; break;
	case MOON_MODULE_ATTR:	break;
	}
}

extern int MoonProjectFindEntityAllNumber(MOON_PROJECTGOD* project)
{
	int all_number = 0;
	printf("\n\033[4;7;105m   序号|地址            |索引      |名称                          |类型                          |Hash      |类型大小  \033[0m\n");
	for (int index = 0; index < ENTITYNUMBER; index++)
		if (project->entityindex[index].length != 0)
		{
			all_number++;
			printf("\033[4;7;105m%-7d|%p|%-10d|%-30s|%-30s|%-10d|%-10d\033[0m\n", 
				all_number, project->entityindex[index].entityindex, index, project->entityindex[index].nameid, 
				project->entityindex[index].type_name, MoonHash(project->entityindex[index].nameid), project->entityindex[index].length);
		}
	printf("\n[ProjectFindEntityAllNumber函数]进入成功!\n统计到的实体总数为[%d]\n", all_number);
	return all_number;
}

extern void MoonPrompt(char* text)
{
	printf("\n\033[31m[MoonEngine]提示\033[0m\n");
	printf("%s\n", text);
}

static _Bool ProjectConsole(MOON_PROJECTGOD* project, int (*developerconsole)(MOON_PROJECTGOD*))
{
	if (developerconsole == MOON_NULL)return MOON_FALSE;
	developerconsole(project);
	return MOON_TRUE;
}

static MOON_PROJECTMODULE(MoonLogicPause)
{
	//printf("[MoonLogicPause函数]进入成功!\n");
	MoonSleep(1);
	return MOON_NOTFOUND;
}

static MOON_PROJECTMODULE(MoonDrawingPause)
{
	//printf("[MoonDrawingPause函数]进入成功!\n");
	MoonSleep(1);
	return MOON_NOTFOUND;
}

extern MOON_MESSAGE_THREAD_TYPE MoonProjectSendMessage(MOON_MESSAGE message, MOON_METADATA metadata)
{
	if (message > MOON_MESSAGE_DRAW_START && message <= MOON_MESSAGE_DRAW_END)
		if (!moon_engine_core.thread_message_type_draw)
		{
			MOON_MESSAGE_SPECIFIC* buffer = (MOON_MESSAGE_SPECIFIC*)realloc(moon_engine_core.message_draw.message, (moon_engine_core.message_draw.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
			if (!buffer)
				return MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE;
			else
			{
				moon_engine_core.message_draw.message = buffer;
				moon_engine_core.message_draw.message[moon_engine_core.message_draw.message_index].message = message;
				moon_engine_core.message_draw.message[moon_engine_core.message_draw.message_index].metadata = metadata;
				moon_engine_core.message_draw.message_index += 1;
				return MOON_MESSAGE_THREAD_TYPE_TRUE;
			}
		}
		else
			return MOON_MESSAGE_THREAD_TYPE_BUSY;

	if (message > MOON_MESSAGE_LOGIC_START && message <= MOON_MESSAGE_LOGIC_END)
		if (!moon_engine_core.thread_message_type_logic)
		{
			MOON_MESSAGE_SPECIFIC* buffer = (MOON_MESSAGE_SPECIFIC*)realloc(moon_engine_core.message_logic.message, (moon_engine_core.message_logic.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
			if (!buffer)
				return MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE;
			else
			{
				moon_engine_core.message_logic.message = buffer;
				moon_engine_core.message_logic.message[moon_engine_core.message_logic.message_index].message = message;
				moon_engine_core.message_logic.message[moon_engine_core.message_logic.message_index].metadata = metadata;
				moon_engine_core.message_logic.message_index += 1;
				return MOON_MESSAGE_THREAD_TYPE_TRUE;
			}
		}
		else
			return MOON_MESSAGE_THREAD_TYPE_BUSY;


	if (message > MOON_MESSAGE_ATTR_START && message <= MOON_MESSAGE_ATTR_END)
		if (!moon_engine_core.thread_message_type_attr)
		{
			MOON_MESSAGE_SPECIFIC* buffer = (MOON_MESSAGE_SPECIFIC*)realloc(moon_engine_core.message_attr.message, (moon_engine_core.message_attr.message_index + 1) * sizeof(MOON_MESSAGE_SPECIFIC));
			if (!buffer)
				return MOON_MESSAGE_THREAD_TYPE_REALLOC_FAILURE;
			else
			{
				moon_engine_core.message_attr.message = buffer;
				moon_engine_core.message_attr.message[moon_engine_core.message_attr.message_index].message = message;
				moon_engine_core.message_attr.message[moon_engine_core.message_attr.message_index].metadata = metadata;
				moon_engine_core.message_attr.message_index += 1;
				return MOON_MESSAGE_THREAD_TYPE_TRUE;
			}
		}
		else
			return MOON_MESSAGE_THREAD_TYPE_BUSY;
}

extern void MoonProjectGetMessage(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type, void(*Handle)(MOON_PROJECTGOD*, MOON_MESSAGE_ALL*, _Bool*))
{
	//type的作用
	//防止随意操作标志导致消息处理紊乱
	*type = MOON_TRUE;
	if (Handle)
	{
		Handle(project, message, type);
		free(message->message);
		//Handle内部应该处理完所有消息,因为该函数结束后,线程循环也差不多结束
		message->message = MOON_NULL;
		message->message_index = 0;
	}
	*type = MOON_FALSE;
	return MOON_FALSE;
}

extern void MoonAttrMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type)
{
	for (int index = 0; index < message->message_index; index++)
	{
		if (*type)
		{
			switch (message->message[index].message)
			{
			case MOON_MESSAGE_ATTR_END: *type = MOON_FALSE; break;
			case MOON_MESSAGE_ATTR_DEAD:
				moon_engine_core.dead = MOON_TRUE;
				break;
			case MOON_MESSAGE_ATTR_POWER:
				moon_engine_core.gamepowermode = message->message[index].metadata.attr.power;
				break;
			case MOON_MESSAGE_ATTR_SETLOGIC:
				MoonProjectFunctionSwitch(MOON_MODULE_LOGIC, message->message[index].metadata.function);
				break;
			case MOON_MESSAGE_ATTR_SETDRAW: 
				MoonProjectFunctionSwitch(MOON_MODULE_DRAW, message->message[index].metadata.function);
				break;
			case MOON_MESSAGE_ATTR_SETFPS: 
			{
				int fps = 1000.f / message->message[index].metadata.attr.fps;
				if (fps <= 0)fps = 1000.f / 60;
				moon_engine_core.timeload.timeload = fps;
			}
			case MOON_MESSAGE_ATTR_OPEN:
			{
				message->message[index].metadata.function_open(project);
			}
			break;
			}
		}
		else return;
	}
}

extern void MoonlogicMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type)
{
	for (int index = 0; index < message->message_index; index++)
	{
		if (*type)
		{
			switch (message->message[index].message)
			{
			case MOON_MESSAGE_LOGIC_END: *type = MOON_FALSE; break;
			//case MOON_MESSAGE_LOGIC_SETLOGIC:				MoonProjectFunctionSwitch(MOON_MODULE_LOGIC, message->message[index].metadata.function);				break;
			case MOON_MESSAGE_LOGIC_OPEN:
			{
				message->message[index].metadata.function_open(project);
			}
			}
		}
		else return;
	}
}

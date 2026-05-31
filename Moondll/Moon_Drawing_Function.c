#define STB_IMAGE_IMPLEMENTATION
#include"MoonCore.h"
//为了方便和防止冲突,所以改名
#include"Moon_stb_image.h"
#include"MoonFontttf.h"

//临时备忘录,发布时需要删除
//需要提供一个直接操作uniform的API

static unsigned int solid_color_shader, texture_shader;
static MOON_IMAGE* moon_engineback;
static unsigned int
moon_vbo_solid, moon_vao_solid,
moon_vbo_texture, moon_vao_texture;
static MOON_POINT3D moon_vertex[MOON_VERTICES_MAX];
static unsigned int moon_vertex_index;
static void MoonVertexinitTemp(MOON_POINT3D* vertex, unsigned int index_offset, float vx, float vy, float r, float g, float b, float a);
static inline _Bool MoonSetTemp(MOON_IMAGE** image_old, MOON_METADATA* metadata, int offset);
static MOON_IMAGE moon_simple_font;

_declspec(dllexport) extern void MoonDrawLoad(MOON_PROJECTGOD* project)
{
	//获取引擎核心着色器
	MoonHashFindEntity(project, "ProjectShader_SolidColor", GLuint, shader_program_1);
	MoonHashFindEntity(project, "ProjectShader_Texture", GLuint, shader_program_2);
	MoonHashFindEntity(project, "ProjectBitmap", MOON_IMAGE, engineback_2);
	moon_engineback = engineback_2;
	solid_color_shader = *shader_program_1;
	texture_shader = *shader_program_2;

	//texture
	{
		glad_glGenVertexArrays(1, &moon_vao_texture);
		glad_glGenBuffers(1, &moon_vbo_texture);

		glad_glBindVertexArray(moon_vao_texture);

		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, NULL, GL_DYNAMIC_DRAW);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);
		glad_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glBindVertexArray(0);
		glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//solid
	{
		glad_glGenVertexArrays(1, &moon_vao_solid);
		glad_glGenBuffers(1, &moon_vbo_solid);

		glad_glBindVertexArray(moon_vao_solid);

		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(MOON_POINT3D) * MOON_VERTICES_MAX, NULL, GL_DYNAMIC_DRAW);

		glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
		glad_glEnableVertexAttribArray(0);
		glad_glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
		glad_glEnableVertexAttribArray(1);

		glad_glBindVertexArray(0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//加载默认纹理
	{
		//创建内部字体

		glad_glGenTextures(1, &moon_simple_font.image.texture);
		glad_glBindTexture(GL_TEXTURE_2D, moon_simple_font.image.texture);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		unsigned char* data = moon_simple_font_data;
		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)MOON_FONT_CHAR_COUNT * MOON_FONT_CHAR_SIZE, (GLsizei)MOON_FONT_CHAR_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glad_glGenFramebuffers(1, &moon_simple_font.image.fbo);
		glad_glBindFramebuffer(GL_FRAMEBUFFER, moon_simple_font.image.fbo);
		glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, moon_simple_font.image.texture, 0);

		moon_simple_font.image_size.w = MOON_FONT_CHAR_COUNT * MOON_FONT_CHAR_SIZE;
		moon_simple_font.image_size.h = MOON_FONT_CHAR_SIZE;
	}
}

_declspec(dllexport) extern void MoonDrawOver()
{
	{
		glad_glDeleteVertexArrays(1, &moon_vao_texture);
		glad_glDeleteBuffers(1, &moon_vbo_texture);
	}

	{
		glad_glDeleteVertexArrays(1, &moon_vao_solid);
		glad_glDeleteBuffers(1, &moon_vbo_solid);
	}

	MoonImageDelete(&moon_simple_font);
}

_declspec(dllexport) extern void MoonShaderLoad(char** vertex_shader, char** pixel_shader, unsigned int* shader_program)
{
	GLint success;

	//編譯頂點著色器
	GLuint vertex_shader_uint = glad_glCreateShader(GL_VERTEX_SHADER);
	glad_glShaderSource(vertex_shader_uint, 1, vertex_shader, MOON_NULL);
	glad_glCompileShader(vertex_shader_uint);
	glad_glGetShaderiv(vertex_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(vertex_shader_uint, 1024, MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, "[MoonRendererLoad]函数错误,顶点着色器编译失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt("顶点着色器编译完成");

	//編譯像素著色器
	GLuint pixel_shader_uint = glad_glCreateShader(GL_FRAGMENT_SHADER);
	glad_glShaderSource(pixel_shader_uint, 1, pixel_shader, MOON_NULL);
	glad_glCompileShader(pixel_shader_uint);
	glad_glGetShaderiv(pixel_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(pixel_shader_uint, 1024, MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, "[MoonRendererLoad]函数错误,像素着色器编译失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt("像素着色器编译完成");


	//組合著色器
	if (*shader_program)
		glad_glDeleteProgram(*shader_program);
	*shader_program = glad_glCreateProgram();
	glad_glAttachShader(*shader_program, vertex_shader_uint);
	glad_glAttachShader(*shader_program, pixel_shader_uint);
	glad_glLinkProgram(*shader_program);
	glad_glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetProgramInfoLog(*shader_program, 1024, MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, "[MoonRendererLoad]函数错误,着色器鏈接失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt("着色器链接完成");


	//刪除著色器對象
	glad_glDeleteShader(vertex_shader_uint);
	glad_glDeleteShader(pixel_shader_uint);
}

_declspec(dllexport) extern void MoonDrawArea(MOON_IMAGE* image_goal, MOON_IMAGE* image_source,int x,int y,int width ,int height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.apx = 0.f;
	metadata.draw.image.apy = 0.f;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.uv_w = 1.f;
	metadata.draw.image.uv_h = 1.f;
	metadata.draw.image.deg = 0;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
}

_declspec(dllexport) extern void MoonDrawAreaRound(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, float apx, float apy, int width, int height, int deg)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.apx = apx;
	metadata.draw.image.apy = apy;
	metadata.draw.image.uv_w = 1.f;
	metadata.draw.image.uv_h = 1.f;
	metadata.draw.image.deg = deg;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
}

_declspec(dllexport) extern void MoonDrawAreaPlgBit(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, MOON_POINT2D point[4])
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image_pig.image_resources = image_source;
	metadata.draw.image_pig.point[0] = point[0];
	metadata.draw.image_pig.point[1] = point[1];
	metadata.draw.image_pig.point[2] = point[2];
	metadata.draw.image_pig.point[3] = point[3];
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_PIG, metadata);
}

_declspec(dllexport) extern void MoonDrawAreaUV(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, int width, int height, float uv_x, float uv_y, float uv_width, float uv_height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image.image_resources = image_source;
	metadata.draw.image.x = x;
	metadata.draw.image.y = y;
	metadata.draw.image.width = width;
	metadata.draw.image.height = height;
	metadata.draw.image.apx = uv_x;
	metadata.draw.image.apy = uv_y;
	metadata.draw.image.uv_w = uv_width;
	metadata.draw.image.uv_h = uv_height;
	metadata.draw.image.deg = 0;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_UV, metadata);
}

_declspec(dllexport) extern void MoonDrawPix(MOON_IMAGE* image, int x, int y, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x;
	metadata.draw.graphic.y1 = y;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_PIX, metadata);
}

_declspec(dllexport) extern void MoonDrawLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
}

_declspec(dllexport) extern void MoonDrawBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y1 = y2;
	metadata.draw.graphic.x2 = x1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y2;
	metadata.draw.graphic.x2 = x1;
	metadata.draw.graphic.y2 = y1;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
}

_declspec(dllexport) extern void MoonDrawTriFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int x3, int y3, unsigned  int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y2;
	metadata.draw.graphic.x3 = x3;
	metadata.draw.graphic.y3 = y3;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
}

_declspec(dllexport) extern void MoonDrawBoxFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y1;
	metadata.draw.graphic.x3 = x1;
	metadata.draw.graphic.y3 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
	metadata.draw.graphic.x1 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TRI_FULL, metadata);
}

_declspec(dllexport) extern int MoonAnimeRun(MOON_IMAGE* image, MOON_ANIME* anime, int animeswitch, int x, int y, int width, int height)
{
	if (!animeswitch)return 0;
	else
	{
		anime->number %= anime->totalnumber;
		MOON_METADATA metadata = { 0 };
		metadata.draw.shader = texture_shader;
		metadata.draw.image_goal = image;
		metadata.draw.image.image_resources = &anime->sequenceframes[anime->number];
		metadata.draw.image.x = x;
		metadata.draw.image.y = y;
		metadata.draw.image.apx = 0.f;
		metadata.draw.image.apy = 0.f;
		metadata.draw.image.uv_w = 1.f;
		metadata.draw.image.uv_h = 1.f;
		metadata.draw.image.width = width;
		metadata.draw.image.height = height;
		metadata.draw.image.deg = 0;
		MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE, metadata);
	}
	if (MoonTimeLoad(&(anime->timeload), 1)) ++anime->number;
	return (int)anime->number;
}

_declspec(dllexport) extern void MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.image_goal = image;
	metadata.draw.text.coord.x = x;
	metadata.draw.text.coord.y = y;
	metadata.draw.text.size_w = sizewidth;
	metadata.draw.text.size_h = sizeheight;
	int len = (int)strlen(text);
	if (len >= MOON_MESSAGE_TEXT_MAX)
	{
		char textbuffer[255] = { 0 };
		snprintf(textbuffer, 255, "单次消息超过最大字符数,或许考虑(拼接|缩短)\n字符串为[%s]", text);
		MoonPrompt((char*)textbuffer);
		return;
	}
	for (int index = 0; index < MOON_MESSAGE_TEXT_MAX; index++)
	{
		char ch = text[index];
		if (ch == '\n' || ch == '\0')
			break;
		metadata.draw.text.text[index] = ch;
	}
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_TEXT, metadata);
}

_declspec(dllexport) extern void MoonImageCreate(MOON_PROJECTGOD* project, MOON_IMAGE* image, int bmpwidth, int bmpheight)
{
	glad_glGenTextures(1, &image->image.texture);
	glad_glBindTexture(GL_TEXTURE_2D, image->image.texture);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int data_size = bmpwidth * bmpheight * 4;
	unsigned char* data = (unsigned char*)calloc(data_size, 1);
	if (data)
	{
		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)bmpwidth, (GLsizei)bmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		free(data);
	}
	else
		MoonPrompt("[MoonImageCreate]函数警告,内存分配失败");
	glad_glGenFramebuffers(1, &image->image.fbo);
	glad_glBindFramebuffer(GL_FRAMEBUFFER, image->image.fbo);
	glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->image.texture, 0);

	image->image_size.w = bmpwidth;
	image->image_size.h = bmpheight;

	char text[255];
	snprintf(text, 255, "创建了一个纹理,值为%d", image->image.texture);
	MoonPrompt(text);
}

_declspec(dllexport) extern void MoonImageDelete(MOON_IMAGE* image)
{
	char text[255];
	snprintf(text, 255, "销毁了一个纹理,值为%d", image->image.texture);
	MoonPrompt(text);
	glad_glDeleteTextures(1, &image->image.texture);
	glad_glDeleteFramebuffers(1, &image->image.fbo);
	image->image.texture = 0;
	image->image.fbo = 0;
	image->image_size.h = 0;
	image->image_size.w = 0;
}

_declspec(dllexport) extern void MoonImageLoad(MOON_IMAGE* image, const char** imagefile, int imagenumber)
{
	for (int index = 0; index < imagenumber; index++)
	{
		unsigned int old_id = image[index].image.texture;
		MOON_IMAGE image_buffer;
		int bmpwidth, bmpheight, bmpchannels;
		glad_glGenTextures(1, &image_buffer.image.texture);
		glad_glBindTexture(GL_TEXTURE_2D, image_buffer.image.texture);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(imagefile[index], &bmpwidth, &bmpheight, &bmpchannels, 4);

		image_buffer.image_size.w = bmpwidth;
		image_buffer.image_size.h = bmpheight;

		glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpwidth, bmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glad_glGenFramebuffers(1, &image_buffer.image.fbo);
		glad_glBindFramebuffer(GL_FRAMEBUFFER, image_buffer.image.fbo);
		glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image_buffer.image.texture, 0);

		stbi_image_free(data);

		MoonImageDelete(&image[index]);
		image[index] = image_buffer;

		char text[255];
		snprintf(text, 255, "加载了一个纹理,值为%d,原值为%d", image[index].image.texture, old_id);
		MoonPrompt(text);
	}
}

_declspec(dllexport) extern void MoonImageLoadBatch(MOON_PROJECTGOD* project, MOON_IMAGE* image, int totalnumber, const char** name, int width, int height)
{
	for (int i = 0; i < totalnumber; i++)
		MoonImageCreate(project, &image[i], width, height);
	MoonImageLoad(image, name, totalnumber);
}

_declspec(dllexport) extern int MoonAnimeInit(MOON_ANIME* anime, MOON_IMAGE* sequenceframes, int timeload, int totalnumber, int width, int height)
{
	if (totalnumber <= 0) { MoonPrompt("[AnimeInit函数错误]动画序列帧总数有问题!\n"); return MOON_Error; }
	if (sequenceframes == NULL) { MoonPrompt("[AnimeInit函数错误]动画序列帧有问题,请检查名为动画是否存在!\n"); return MOON_Error; }
	anime->sequenceframes = sequenceframes;
	anime->totalnumber = totalnumber;
	anime->number = 0;
	for (int i = 0; i < anime->totalnumber; i++)
	{
		anime->sequenceframes[i].image_size.x = width;
		anime->sequenceframes[i].image_size.y = height;
	}
	MoonTimeLoadInit(&(anime->timeload), timeload);		//设置定时器
	return MOON_YES;
}

_declspec(dllexport) extern void MoonAnimeDelete(MOON_ANIME* anime)
{
	if (anime == MOON_NULL) return;
	for (int i = 0; i < anime->totalnumber; i++)
		MoonImageDelete(&anime->sequenceframes[i]);
	anime->sequenceframes = MOON_NULL;
	anime->totalnumber = 0;
	anime->number = 0;
}

_declspec(dllexport) extern void MoonAnimeCreate(
	MOON_PROJECTGOD* project, MOON_IMAGE* image, MOON_ANIME* anime,
	int totalnumber, const char** animename, char* entityname,
	int timeload, int width, int height)
{
	MoonImageLoadBatch(project, image, totalnumber, animename, width, height);
	MoonAnimeInit(anime, image, timeload, totalnumber, width, height);
	MoonCreateEntityIndex(project, anime, entityname, sizeof(MOON_ANIME), "MOON_ANIME");
}

_declspec(dllexport) extern void MoonImageDesignated(MOON_IMAGE* image)
{
	//glad_glBindFramebuffer	紋理作爲畫布(寫入)
	//glad_glBindTexture		紋理作爲資源(讀取)
	glad_glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)image->image.fbo);
}

_declspec(dllexport) extern void MoonImageShader(unsigned int shader)
{
	glad_glUseProgram(shader);
}

_declspec(dllexport) extern void MoonCoreDrawArea(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal || !metadata->draw.image.image_resources)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}

	float
		vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image.x * 1.f / metadata->draw.image_goal->image_size.w),
		vy1 = MoonLerp(1.f, -1.f, metadata->draw.image.y * 1.f / metadata->draw.image_goal->image_size.h),
		vx2 = MoonLerp(-1.f, 1.f, (metadata->draw.image.x + metadata->draw.image.image_resources->image_size.w) * 1.f / metadata->draw.image_goal->image_size.w),
		vy2 = MoonLerp(1.f, -1.f, (metadata->draw.image.y + metadata->draw.image.image_resources->image_size.h) * 1.f / metadata->draw.image_goal->image_size.h);
	float
		vx3 = vx1, 
		vy3 = vy2,
		vx4 = vx2, 
		vy4 = vy1;

	{
		//这里处理旋转

		float 
			cosrad = (float)cos(MoonDegRad(metadata->draw.image.deg)), 
			sinrad = (float)sin(MoonDegRad(metadata->draw.image.deg));
		MOON_POINT2D points[4];
		int apx = (int)(-metadata->draw.image.apx * metadata->draw.image.image_resources->image_size.w),
			apy = (int)(-metadata->draw.image.apy * metadata->draw.image.image_resources->image_size.h);
		float matrix2d[4] = { cosrad,-sinrad,sinrad,cosrad };
		points[0].x = (long int)(metadata->draw.image.x + matrix2d[0] * apx + matrix2d[2] * apy);
		points[0].y = (long int)(metadata->draw.image.y + matrix2d[1] * apx + matrix2d[3] * apy);																		//0,0
		points[1].x = (long int)(metadata->draw.image.x + matrix2d[0] * (apx + metadata->draw.image.width) + matrix2d[2] * apy);
		points[1].y = (long int)(metadata->draw.image.y + matrix2d[1] * (apx + metadata->draw.image.width) + matrix2d[3] * apy);										//1,0
		points[2].x = (long int)(metadata->draw.image.x + matrix2d[0] * apx + matrix2d[2] * (apy + metadata->draw.image.height));
		points[2].y = (long int)(metadata->draw.image.y + matrix2d[1] * apx + matrix2d[3] * (apy + metadata->draw.image.height));										//0,1
		points[3].x = (long int)(metadata->draw.image.x + matrix2d[0] * (apx + metadata->draw.image.width) + matrix2d[2] * (apy + metadata->draw.image.height));
		points[3].y = (long int)(metadata->draw.image.y + matrix2d[1] * (apx + metadata->draw.image.width) + matrix2d[3] * (apy + metadata->draw.image.height));		//1,1
		
		vx1 = MoonLerp(-1.f, 1.f, points[0].x * 1.f / metadata->draw.image_goal->image_size.w);
		vy1 = MoonLerp(1.f, -1.f, points[0].y * 1.f / metadata->draw.image_goal->image_size.h);
		vx2 = MoonLerp(-1.f, 1.f, points[3].x * 1.f / metadata->draw.image_goal->image_size.w);
		vy2 = MoonLerp(1.f, -1.f, points[3].y * 1.f / metadata->draw.image_goal->image_size.h);
		vx3 = MoonLerp(-1.f, 1.f, points[2].x * 1.f / metadata->draw.image_goal->image_size.w);
		vy3 = MoonLerp(1.f, -1.f, points[2].y * 1.f / metadata->draw.image_goal->image_size.h);
		vx4 = MoonLerp(-1.f, 1.f, points[1].x * 1.f / metadata->draw.image_goal->image_size.w);
		vy4 = MoonLerp(1.f, -1.f, points[1].y * 1.f / metadata->draw.image_goal->image_size.h);
	}

	float vertexs[20] =
	{
		vx1, vy1, 0.f, 0.f, 1.f,
		vx4, vy4, 0.f, 1.f, 1.f,
		vx3, vy3, 0.f, 0.f, 0.f,
		vx2, vy2, 0.f, 1.f, 0.f,
	};

	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, metadata->draw.image.image_resources->image.texture);
		glad_glUniform1i(glGetUniformLocation(metadata->draw.shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexs), vertexs);
		glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCoreDrawAreaUV(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal || !metadata->draw.image.image_resources)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}
	MOON_POINT2D image_buffer_size =
	{
		(long int)(metadata->draw.image.image_resources->image_size.w * metadata->draw.image.uv_w),
		(long int)(metadata->draw.image.image_resources->image_size.h * metadata->draw.image.uv_h)
	};

	metadata->draw.image.apy = 1.f - metadata->draw.image.apy;

	float
		vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image.x * 1.f / metadata->draw.image_goal->image_size.w),
		vy1 = MoonLerp(1.f, -1.f, metadata->draw.image.y * 1.f / metadata->draw.image_goal->image_size.h),
		vx2 = MoonLerp(-1.f, 1.f, (metadata->draw.image.x + image_buffer_size.w + metadata->draw.image.width) * 1.f / metadata->draw.image_goal->image_size.w),
		vy2 = MoonLerp(1.f, -1.f, (metadata->draw.image.y + image_buffer_size.h + +metadata->draw.image.height) * 1.f / metadata->draw.image_goal->image_size.h),
		uv_w = metadata->draw.image.uv_w,
		uv_h = metadata->draw.image.uv_h;

	float
		uv_left = MoonRange(metadata->draw.image.apx, 0, 1.f),
		uv_right = MoonRange(metadata->draw.image.apx + uv_w, 0, 1.f),
		uv_top = MoonRange(metadata->draw.image.apy, 0, 1.f),
		uv_bottom = MoonRange(metadata->draw.image.apy - uv_h, 0, 1.f);

	float vertexs[20] =
	{
		vx1, vy2, 0.f, uv_left,		uv_bottom,
		vx2, vy2, 0.f, uv_right,	uv_bottom,
		vx1, vy1, 0.f, uv_left,		uv_top,
		vx2, vy1, 0.f, uv_right,	uv_top,
	};

	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, metadata->draw.image.image_resources->image.texture);
		glad_glUniform1i(glGetUniformLocation(metadata->draw.shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexs), vertexs);
		glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCoreDrawAreaPlgBit(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal || !metadata->draw.image.image_resources)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}
	float
		vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[2].x * 1.f / metadata->draw.image_goal->image_size.w),
		vy1 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[2].y * 1.f / metadata->draw.image_goal->image_size.h),
		vx2 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[1].x * 1.f / metadata->draw.image_goal->image_size.w),
		vy2 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[1].y * 1.f / metadata->draw.image_goal->image_size.h),
		vx3 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[0].x * 1.f / metadata->draw.image_goal->image_size.w),
		vy3 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[0].y * 1.f / metadata->draw.image_goal->image_size.h),
		vx4 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[3].x * 1.f / metadata->draw.image_goal->image_size.w),
		vy4 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[3].y * 1.f / metadata->draw.image_goal->image_size.h);

	float vertexs[20] =
	{
		vx1, vy1, 0.f, 0.f, 0.f,
		vx4, vy4, 0.f, 1.f, 0.f,
		vx3, vy3, 0.f, 0.f, 1.f,
		vx2, vy2, 0.f, 1.f, 1.f,
	};

	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, metadata->draw.image.image_resources->image.texture);
		glad_glUniform1i(glGetUniformLocation(metadata->draw.shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexs), vertexs);
		glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCoreLines(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}
	
	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_LINES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCorePixs(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}

	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_POINTS, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCoreTriFulls(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}
	
	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_TRIANGLES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

_declspec(dllexport) extern void MoonCoreFont(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt("无效的纹理对象");
		return;
	}
	for (int index = 0; index < MOON_MESSAGE_TEXT_MAX; index++)
	{
		char ch = metadata->draw.text.text[index];

		MOON_POINT2D image_buffer_size =
		{
			(long int)(metadata->draw.text.size_w),
			(long int)(metadata->draw.text.size_h),
		};
		int x = image_buffer_size.w * index + metadata->draw.text.coord.x;

		float font_w = 1.f / MOON_FONT_CHAR_COUNT;
		float
			vx1 = MoonLerp(-1.f, 1.f, x * 1.f / metadata->draw.image_goal->image_size.w),
			vy1 = MoonLerp(1.f, -1.f, metadata->draw.text.coord.y * 1.f / metadata->draw.image_goal->image_size.h),
			vx2 = MoonLerp(-1.f, 1.f, (x + image_buffer_size.w) * 1.f / metadata->draw.image_goal->image_size.w),
			vy2 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h) * 1.f / metadata->draw.image_goal->image_size.h),
			uv_w = font_w,
			uv_h = 1.f,
			apx = ch * font_w;

		float
			uv_left = MoonRange(apx, 0, 1.f),
			uv_right = MoonRange(apx + uv_w, 0, 1.f),
			uv_top = 0.f,
			uv_bottom = 1.f;

		float vertexs[20] =
		{
			vx1, vy2, 0.f, uv_left,		uv_bottom,
			vx2, vy2, 0.f, uv_right,	uv_bottom,
			vx1, vy1, 0.f, uv_left,		uv_top,
			vx2, vy1, 0.f, uv_right,	uv_top,
		};

		MoonImageShader(texture_shader);

		{
			glad_glBindVertexArray(moon_vao_texture);
			glad_glBindTexture(GL_TEXTURE_2D, moon_simple_font.image.texture);
			glad_glUniform1i(glGetUniformLocation(texture_shader, "moon_utexture"), 0);
			glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
			glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexs), vertexs);
			glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
			glad_glBindVertexArray(0);
		}
	}
}

_declspec(dllexport) extern void MoonDrawMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type)
{
	MOON_IMAGE* image_old = MOON_NULL;
	for (int index = 0; (unsigned int)index < message->message_index; index++)
		if (*type)
		{
			switch (message->message[index].message)
			{
			case MOON_MESSAGE_DRAW_END: *type = MOON_FALSE; break;
			//case MOON_MESSAGE_DRAW_SETDRAW:				MoonProjectFunctionSwitch(MOON_MODULE_DRAW, message->message[index].metadata.function);			break;

			case MOON_MESSAGE_DRAW_IMAGE:
			{
				MOON_POINT2D old_size = message->message[index].metadata.draw.image.image_resources->image_size;
				message->message[index].metadata.draw.image.image_resources->image_size.w = message->message[index].metadata.draw.image.width;
				message->message[index].metadata.draw.image.image_resources->image_size.h = message->message[index].metadata.draw.image.height;
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				MoonCoreDrawArea(&message->message[index].metadata);
				message->message[index].metadata.draw.image.image_resources->image_size = old_size;
			}
			break;

			case MOON_MESSAGE_DRAW_IMAGE_UV:
			{
				MOON_POINT2D old_size = message->message[index].metadata.draw.image.image_resources->image_size;
				message->message[index].metadata.draw.image.image_resources->image_size.w = message->message[index].metadata.draw.image.width;
				message->message[index].metadata.draw.image.image_resources->image_size.h = message->message[index].metadata.draw.image.height;
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				MoonCoreDrawAreaUV(&message->message[index].metadata);
				message->message[index].metadata.draw.image.image_resources->image_size = old_size;
			}
			break;
			
			case MOON_MESSAGE_DRAW_IMAGE_PIG:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				MoonCoreDrawAreaPlgBit(&message->message[index].metadata);
			}
			break;
			
			case MOON_MESSAGE_DRAW_IMAGE_CLEAN:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) / 255.f,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) / 255.f,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) / 255.f,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) / 255.f;
				glad_glClearColor(r, g, b, a);
				glad_glClear(GL_COLOR_BUFFER_BIT);
			}
			break;
			
			case MOON_MESSAGE_DRAW_PIX:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 1))
					break;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) / 255.f,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) / 255.f,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) / 255.f,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) / 255.f,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h);

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r, g, b, a);

				moon_vertex_index += 1;

				if (
					(index == message->message_index - 1)
					|| message->message[index + 1].message != MOON_MESSAGE_DRAW_PIX
					|| message->message[index + 1].metadata.draw.image_goal != image_old)
				{
					MoonCorePixs(moon_vertex, moon_vertex_index, &message->message[index].metadata);
					moon_vertex_index = 0;
				}
			}
			break;

			case MOON_MESSAGE_DRAW_LINE:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 2))
					break;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) / 255.f,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) / 255.f,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) / 255.f,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) / 255.f,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h),
					vx2 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x2 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy2 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y2 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h);

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r, g, b, a);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r, g, b, a);

				moon_vertex_index += 2;

				if (
					(index == message->message_index - 1)
					|| message->message[index + 1].message != MOON_MESSAGE_DRAW_LINE
					|| message->message[index + 1].metadata.draw.image_goal != image_old)
				{
					MoonCoreLines(moon_vertex, moon_vertex_index, &message->message[index].metadata);
					moon_vertex_index = 0;
				}
			}
			break;

			case MOON_MESSAGE_DRAW_TRI_FULL:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 3))
					break;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) / 255.f,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) / 255.f,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) / 255.f,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) / 255.f,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h),
					vx2 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x2 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy2 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y2 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h),
					vx3 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x3 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy3 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y3 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h);

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r, g, b, a);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r, g, b, a);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 2, vx3, vy3, r, g, b, a);

				moon_vertex_index += 3;

				if (
					(index == message->message_index - 1)
					|| message->message[index + 1].message != MOON_MESSAGE_DRAW_TRI_FULL
					|| message->message[index + 1].metadata.draw.image_goal != image_old)
				{
					MoonCoreTriFulls(moon_vertex, moon_vertex_index, &message->message[index].metadata);
					moon_vertex_index = 0;
				}
			}
			break;

			case MOON_MESSAGE_DRAW_TEXT:
			{
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				MoonCoreFont(&message->message[index].metadata);
			}
			break;
			
			case MOON_MESSAGE_DRAW_OPEN:
			{
				message->message[index].metadata.function_open(project);
			}
			break;


			}
		}
		else return;
}

//此函数仅作为辅助函数
static inline void MoonVertexinitTemp(MOON_POINT3D* vertex, unsigned int index_offset, float vx, float vy, float r, float g, float b, float a)
{
	vertex[index_offset].x = vx;
	vertex[index_offset].y = vy;
	vertex[index_offset].z = 0.f;
	vertex[index_offset].a = a;
	vertex[index_offset].r = r;
	vertex[index_offset].g = g;
	vertex[index_offset].b = b;
}

//此函数仅作为辅助函数
static inline _Bool MoonSetTemp(MOON_IMAGE** image_old, MOON_METADATA* metadata, int offset)
{
	MOON_IMAGE* image_new = metadata->draw.image_goal;

	if (*image_old != image_new)
	{
		MoonImageDesignated(image_new);
		*image_old = image_new;
	}
	{
		static MOON_POINT2D view;
		if (view.w != image_new->image_size.w
			|| view.h != image_new->image_size.h)
		{
			glad_glViewport(0, 0, image_new->image_size.w, image_new->image_size.h);
			view.w = image_new->image_size.w;
			view.h = image_new->image_size.h;
		}
	}
	if (moon_vertex_index >= (unsigned int)MOON_VERTICES_MAX - offset)
	{
		MoonPrompt("顶点溢出");
		return MOON_FALSE;
	}
	return MOON_TRUE;
}

#define STB_IMAGE_IMPLEMENTATION
#include"MoonCore.h"
//为了方便和防止冲突,所以改名
#include"Moon_stb_image.h"
#include"MoonFontttf.h"

//临时备忘录
//需要提供一个直接操作uniform的API

static MOON_IMAGE* moon_engineback;
static MOON_IMAGE moon_simple_font;
static MOON_POINT3D moon_vertex[MOON_VERTICES_MAX];
static MOON_TEXTURE_VECTER moon_vertex_texture[MOON_VERTICES_MAX];
static unsigned int
solid_color_shader, texture_shader,
moon_vbo_solid, moon_vao_solid,
moon_vbo_texture, moon_vao_texture,
moon_vertex_index, moon_vertex_texture_index;
static void MoonVertexinitTemp(MOON_POINT3D* vertex, unsigned int index_offset, float vx, float vy, float r, float g, float b, float a);
static inline void MoonTextureVertexinitTemp(MOON_TEXTURE_VECTER* vertex, unsigned int index_offset, float vx, float vy, float uv_x, float uv_y);
static inline _Bool MoonSetTemp(MOON_IMAGE** image_old, MOON_METADATA* metadata, int offset);
static void MoonDrawAreaTemp(unsigned int message_type, unsigned int index, MOON_MESSAGE_ALL* message, MOON_IMAGE* image_old, MOON_IMAGE* image_resource_old);

extern void MoonDrawLoad(MOON_PROJECTGOD* project)
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
		glad_glBufferData(GL_ARRAY_BUFFER, sizeof(MOON_TEXTURE_VECTER) * MOON_VERTICES_MAX, NULL, GL_DYNAMIC_DRAW);

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

extern void MoonDrawOver()
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

extern void MoonShaderLoad(char** vertex_shader, char** pixel_shader, unsigned int* shader_program)
{
	GLint success;

	//編譯頂點著色器
	GLuint vertex_shader_uint = glad_glCreateShader(GL_VERTEX_SHADER);
	glad_glShaderSource(vertex_shader_uint, 1, vertex_shader, (const GLint*)MOON_NULL);
	glad_glCompileShader(vertex_shader_uint);
	glad_glGetShaderiv(vertex_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(vertex_shader_uint, 1024, (GLsizei*)MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,顶点着色器编译失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt((char*)"顶点着色器编译完成");

	//編譯像素著色器
	GLuint pixel_shader_uint = glad_glCreateShader(GL_FRAGMENT_SHADER);
	glad_glShaderSource(pixel_shader_uint, 1, pixel_shader, (const GLint*)MOON_NULL);
	glad_glCompileShader(pixel_shader_uint);
	glad_glGetShaderiv(pixel_shader_uint, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char error_log[1024];
		glad_glGetShaderInfoLog(pixel_shader_uint, 1024, (GLsizei*)MOON_NULL, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,像素着色器编译失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt((char*)"像素着色器编译完成");


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
		glad_glGetProgramInfoLog(*shader_program, 1024, 0, error_log);
		MoonPrompt(error_log);
		MoonProjectError(shader_program, 1, (char*)"[MoonRendererLoad]函数错误,着色器鏈接失败!");
		MOON_METADATA metadata = { 0 };
		MoonProjectSendMessage(MOON_MESSAGE_DEAD, metadata);
		return;
	}
	success = 0;
	MoonPrompt((char*)"着色器链接完成");


	//刪除著色器對象
	glad_glDeleteShader(vertex_shader_uint);
	glad_glDeleteShader(pixel_shader_uint);
}

extern void MoonDrawArea(MOON_IMAGE* image_goal, MOON_IMAGE* image_source,int x,int y,int width ,int height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
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

extern void MoonDrawAreaRound(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, float apx, float apy, int width, int height, int deg)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
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

extern void MoonDrawAreaPlgBit(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, MOON_POINT2D point[4])
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.shader = texture_shader;
	metadata.draw.image_goal = image_goal;
	metadata.draw.image_pig.image_resources = image_source;
	metadata.draw.image_pig.point[0] = point[0];
	metadata.draw.image_pig.point[1] = point[1];
	metadata.draw.image_pig.point[2] = point[2];
	metadata.draw.image_pig.point[3] = point[3];
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_IMAGE_PIG, metadata);
}

extern void MoonDrawAreaUV(MOON_IMAGE* image_goal, MOON_IMAGE* image_source, int x, int y, int width, int height, float uv_x, float uv_y, float uv_width, float uv_height)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
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

extern void MoonDrawPix(MOON_IMAGE* image, int x, int y, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x;
	metadata.draw.graphic.y1 = y;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_PIX, metadata);
}

extern void MoonDrawLine(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.image_goal = image;
	metadata.draw.shader = solid_color_shader;
	metadata.draw.graphic.x1 = x1;
	metadata.draw.graphic.y1 = y1;
	metadata.draw.graphic.x2 = x2;
	metadata.draw.graphic.y2 = y2;
	MoonProjectSendMessage(MOON_MESSAGE_DRAW_LINE, metadata);
}

extern void MoonDrawBox(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
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

extern void MoonDrawTriFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, int x3, int y3, unsigned  int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.graphic.color_3 = color;
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

extern void MoonDrawBoxFull(MOON_IMAGE* image, int x1, int y1, int x2, int y2, unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = 0xffffffff;
	metadata.draw.graphic.color_1 = color;
	metadata.draw.graphic.color_2 = color;
	metadata.draw.graphic.color_3 = color;
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

extern int MoonAnimeRun(MOON_IMAGE* image, MOON_ANIME* anime, int animeswitch, int x, int y, int width, int height)
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

extern void MoonDrawTextFont(MOON_IMAGE* image, const char* text, int x, int y, int sizewidth, int sizeheight,unsigned int color)
{
	MOON_METADATA metadata = { 0 };
	metadata.draw.color = color;
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

extern void MoonImageCreate(MOON_IMAGE* image, int bmpwidth, int bmpheight)
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
		MoonPrompt((char*)"[MoonImageCreate]函数警告,内存分配失败");
	glad_glGenFramebuffers(1, &image->image.fbo);
	glad_glBindFramebuffer(GL_FRAMEBUFFER, image->image.fbo);
	glad_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->image.texture, 0);

	image->image_size.w = bmpwidth;
	image->image_size.h = bmpheight;

	char text[255];
	snprintf(text, 255, "创建了一个纹理,值为%d", image->image.texture);
	MoonPrompt(text);
}

extern void MoonImageDelete(MOON_IMAGE* image)
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

extern void MoonImageLoad(MOON_IMAGE* image, const char** imagefile, int imagenumber)
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

extern void MoonImageLoadBatch(MOON_IMAGE* image, int totalnumber, const char** name, int width, int height)
{
	for (int i = 0; i < totalnumber; i++)
		MoonImageCreate(&image[i], width, height);
	MoonImageLoad(image, name, totalnumber);
}

extern int MoonAnimeInit(MOON_ANIME* anime, MOON_IMAGE* sequenceframes, int timeload, int totalnumber, int width, int height)
{
	if (totalnumber <= 0) { MoonPrompt((char*)"[AnimeInit函数错误]动画序列帧总数有问题!\n"); return MOON_Error; }
	if (sequenceframes == NULL) { MoonPrompt((char*)"[AnimeInit函数错误]动画序列帧有问题,请检查名为动画是否存在!\n"); return MOON_Error; }
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

extern void MoonAnimeDelete(MOON_ANIME* anime)
{
	if (anime == MOON_NULL) return;
	for (int i = 0; i < anime->totalnumber; i++)
		MoonImageDelete(&anime->sequenceframes[i]);
	anime->sequenceframes = (MOON_IMAGE*)MOON_NULL;
	anime->totalnumber = 0;
	anime->number = 0;
}

extern void MoonAnimeCreate(
	MOON_PROJECTGOD* project, MOON_IMAGE* image, MOON_ANIME* anime,
	int totalnumber, const char** animename, char* entityname,
	int timeload, int width, int height)
{
	MoonImageLoadBatch(image, totalnumber, animename, width, height);
	MoonAnimeInit(anime, image, timeload, totalnumber, width, height);
	MoonCreateEntityIndex(project, anime, entityname, sizeof(MOON_ANIME), (char*)"MOON_ANIME");
}

extern void MoonImageDesignated(MOON_IMAGE* image)
{
	//glad_glBindFramebuffer	紋理作爲畫布(寫入)
	//glad_glBindTexture		紋理作爲資源(讀取)
	glad_glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)image->image.fbo);
}

extern void MoonImageShader(unsigned int shader)
{
	glad_glUseProgram(shader);
}

extern void MoonCoreDrawArea(MOON_TEXTURE_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal || !metadata->draw.image.image_resources)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}

	MoonImageShader(metadata->draw.shader);

	{
		float
			r = ((metadata->draw.color >> 0) & 0xFF) / 255.f,
			g = ((metadata->draw.color >> 8) & 0xFF) / 255.f,
			b = ((metadata->draw.color >> 16) & 0xFF) / 255.f,
			a = ((metadata->draw.color >> 24) & 0xFF) / 255.f;
		glad_glBindVertexArray(moon_vao_texture);
		glad_glBindTexture(GL_TEXTURE_2D, metadata->draw.image.image_resources->image.texture);
		glad_glUniform4f(glad_glGetUniformLocation(metadata->draw.shader, "moon_ucolor"), r, g, b, a);
		glad_glUniform1i(glad_glGetUniformLocation(metadata->draw.shader, "moon_utexture"), 0);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_TEXTURE_VECTER) * vertex_number, vertexs);
		glad_glDrawArrays(GL_TRIANGLES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

extern void MoonCoreLines(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}
	
	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_LINES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

extern void MoonCorePixs(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}

	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_POINTS, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

extern void MoonCoreTriFulls(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}
	
	MoonImageShader(metadata->draw.shader);

	{
		glad_glBindVertexArray(moon_vao_solid);
		glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_solid);
		glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MOON_POINT3D) * vertex_number, vertexs);
		glad_glDrawArrays(GL_TRIANGLES, 0, vertex_number);
		glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
		glad_glBindVertexArray(0);
	}
}

extern void MoonCoreFont(MOON_METADATA* metadata)
{
	if (!metadata->draw.image_goal)
	{
		MoonPrompt((char*)"无效的纹理对象");
		return;
	}

	float
		r = ((metadata->draw.color >> 0) & 0xFF) / 255.f,
		g = ((metadata->draw.color >> 8) & 0xFF) / 255.f,
		b = ((metadata->draw.color >> 16) & 0xFF) / 255.f,
		a = ((metadata->draw.color >> 24) & 0xFF) / 255.f,
		font_w = 1.f / MOON_FONT_CHAR_COUNT,
		uv_h = 1.f;

	MOON_POINT2D image_buffer_size =
	{
		(long int)(metadata->draw.text.size_w),
		(long int)(metadata->draw.text.size_h),
	};

	for (int index = 0; index < MOON_MESSAGE_TEXT_MAX; index++)
	{
		int ch = metadata->draw.text.text[index],
			x = image_buffer_size.w * index + metadata->draw.text.coord.x;

		float
			vx1 = MoonLerp(-1.f, 1.f, x * 1.f / metadata->draw.image_goal->image_size.w),
			vy1 = MoonLerp(1.f, -1.f, metadata->draw.text.coord.y * 1.f / metadata->draw.image_goal->image_size.h),
			vx2 = MoonLerp(-1.f, 1.f, (x + image_buffer_size.w) * 1.f / metadata->draw.image_goal->image_size.w),
			vy2 = MoonLerp(1.f, -1.f, (metadata->draw.text.coord.y + image_buffer_size.h) * 1.f / metadata->draw.image_goal->image_size.h),
			uv_w = font_w,
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
			glad_glUniform4f(glad_glGetUniformLocation(texture_shader, "moon_ucolor"), r, g, b, a);
			glad_glUniform1i(glad_glGetUniformLocation(texture_shader, "moon_utexture"), 0);
			glad_glBindBuffer(GL_ARRAY_BUFFER, moon_vbo_texture);
			glad_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexs), vertexs);
			glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glad_glBindBuffer(GL_ARRAY_BUFFER, 0);
			glad_glBindVertexArray(0);
		}
	}
}

extern void MoonDrawMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type)
{
	MOON_IMAGE* image_old = (MOON_IMAGE*)MOON_NULL;
	MOON_IMAGE* image_resource_old = (MOON_IMAGE*)MOON_NULL;
	for (unsigned int index = 0; index < message->message_index; index++)
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
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{

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
						float
							buffer_x = metadata->draw.image.x + matrix2d[0],
							buffer_y = metadata->draw.image.y + matrix2d[1];
						points[0].x = (long int)(buffer_x * apx + matrix2d[2] * apy);
						points[0].y = (long int)(buffer_y * apx + matrix2d[3] * apy);																		//0,0
						points[1].x = (long int)(buffer_x * (apx + metadata->draw.image.width) + matrix2d[2] * apy);
						points[1].y = (long int)(buffer_y * (apx + metadata->draw.image.width) + matrix2d[3] * apy);										//1,0
						points[2].x = (long int)(buffer_x * apx + matrix2d[2] * (apy + metadata->draw.image.height));
						points[2].y = (long int)(buffer_y * apx + matrix2d[3] * (apy + metadata->draw.image.height));										//0,1
						points[3].x = (long int)(buffer_x * (apx + metadata->draw.image.width) + matrix2d[2] * (apy + metadata->draw.image.height));
						points[3].y = (long int)(buffer_y * (apx + metadata->draw.image.width) + matrix2d[3] * (apy + metadata->draw.image.height));		//1,1

						vx1 = MoonLerp(-1.f, 1.f, points[0].x * 1.f / metadata->draw.image_goal->image_size.w);
						vy1 = MoonLerp(1.f, -1.f, points[0].y * 1.f / metadata->draw.image_goal->image_size.h);
						vx2 = MoonLerp(-1.f, 1.f, points[3].x * 1.f / metadata->draw.image_goal->image_size.w);
						vy2 = MoonLerp(1.f, -1.f, points[3].y * 1.f / metadata->draw.image_goal->image_size.h);
						vx3 = MoonLerp(-1.f, 1.f, points[2].x * 1.f / metadata->draw.image_goal->image_size.w);
						vy3 = MoonLerp(1.f, -1.f, points[2].y * 1.f / metadata->draw.image_goal->image_size.h);
						vx4 = MoonLerp(-1.f, 1.f, points[1].x * 1.f / metadata->draw.image_goal->image_size.w);
						vy4 = MoonLerp(1.f, -1.f, points[1].y * 1.f / metadata->draw.image_goal->image_size.h);
					}

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy1, 0.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy2, 1.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx4, vy4, 1.f, 1.f);
				}

				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE, index, message, image_old, image_resource_old);

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
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{
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

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy2, uv_left, uv_bottom);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx2, vy2, uv_right, uv_bottom);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx1, vy1, uv_left, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy1, uv_right, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx1, vy1, uv_left, uv_top);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx2, vy2, uv_right, uv_bottom);
				}

				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE_UV, index, message, image_old, image_resource_old);

				message->message[index].metadata.draw.image.image_resources->image_size = old_size;
			}
			break;
			
			case MOON_MESSAGE_DRAW_IMAGE_PIG:
			{
				MOON_POINT2D old_size = message->message[index].metadata.draw.image.image_resources->image_size;
				message->message[index].metadata.draw.image.image_resources->image_size.w = message->message[index].metadata.draw.image.width;
				message->message[index].metadata.draw.image.image_resources->image_size.h = message->message[index].metadata.draw.image.height;
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;
				image_resource_old = message->message[index].metadata.draw.image.image_resources;

				MOON_METADATA* metadata = &message->message[index].metadata;

				{
					float
						vx1 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[2].x * 1.f / metadata->draw.image_goal->image_size.w),
						vy1 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[2].y * 1.f / metadata->draw.image_goal->image_size.h),
						vx2 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[1].x * 1.f / metadata->draw.image_goal->image_size.w),
						vy2 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[1].y * 1.f / metadata->draw.image_goal->image_size.h),
						vx3 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[0].x * 1.f / metadata->draw.image_goal->image_size.w),
						vy3 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[0].y * 1.f / metadata->draw.image_goal->image_size.h),
						vx4 = MoonLerp(-1.f, 1.f, metadata->draw.image_pig.point[3].x * 1.f / metadata->draw.image_goal->image_size.w),
						vy4 = MoonLerp(1.f, -1.f, metadata->draw.image_pig.point[3].y * 1.f / metadata->draw.image_goal->image_size.h);

					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 0, vx1, vy1, 0.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 1, vx4, vy4, 1.f, 1.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 2, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 3, vx2, vy2, 1.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 4, vx3, vy3, 0.f, 0.f);
					MoonTextureVertexinitTemp(moon_vertex_texture, moon_vertex_texture_index + 5, vx4, vy4, 1.f, 1.f);
				}
			
				moon_vertex_texture_index += 6;

				MoonDrawAreaTemp(MOON_MESSAGE_DRAW_IMAGE_PIG, index, message, image_old, image_resource_old);

				message->message[index].metadata.draw.image.image_resources->image_size = old_size;
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
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
					break;

				float
					r = ((message->message[index].metadata.draw.color >> 0) & 0xFF) / 255.f,
					g = ((message->message[index].metadata.draw.color >> 8) & 0xFF) / 255.f,
					b = ((message->message[index].metadata.draw.color >> 16) & 0xFF) / 255.f,
					a = ((message->message[index].metadata.draw.color >> 24) & 0xFF) / 255.f,
					vx1 = MoonLerp(-1.f, 1.f, message->message[index].metadata.draw.graphic.x1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.w),
					vy1 = MoonLerp(1.f, -1.f, message->message[index].metadata.draw.graphic.y1 * 1.f / message->message[index].metadata.draw.image_goal->image_size.h);

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) / 255.f * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) / 255.f * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) / 255.f * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) / 255.f * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);

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
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
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

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) / 255.f * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) / 255.f * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) / 255.f * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) / 255.f * a,
					r_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 0) & 0xFF) / 255.f * r,
					g_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 8) & 0xFF) / 255.f * g,
					b_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 16) & 0xFF) / 255.f * b,
					a_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 24) & 0xFF) / 255.f * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r_2, g_2, b_2, a_2);

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
				if (!MoonSetTemp(&image_old, &message->message[index].metadata, 0))
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

				float
					r_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 0) & 0xFF) / 255.f * r,
					g_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 8) & 0xFF) / 255.f * g,
					b_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 16) & 0xFF) / 255.f * b,
					a_1 = ((message->message[index].metadata.draw.graphic.color_1 >> 24) & 0xFF) / 255.f * a,
					r_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 0) & 0xFF) / 255.f * r,
					g_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 8) & 0xFF) / 255.f * g,
					b_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 16) & 0xFF) / 255.f * b,
					a_2 = ((message->message[index].metadata.draw.graphic.color_2 >> 24) & 0xFF) / 255.f * a,
					r_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 0) & 0xFF) / 255.f * r,
					g_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 8) & 0xFF) / 255.f * g,
					b_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 16) & 0xFF) / 255.f * b,
					a_3 = ((message->message[index].metadata.draw.graphic.color_3 >> 24) & 0xFF) / 255.f * a;

				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 0, vx1, vy1, r_1, g_1, b_1, a_1);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 1, vx2, vy2, r_2, g_2, b_2, a_2);
				MoonVertexinitTemp(moon_vertex, moon_vertex_index + 2, vx3, vy3, r_3, g_3, b_3, a_3);

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
static inline void MoonTextureVertexinitTemp(MOON_TEXTURE_VECTER* vertex, unsigned int index_offset, float vx, float vy, float uv_x, float uv_y)
{
	vertex[index_offset].x = vx;
	vertex[index_offset].y = vy;
	vertex[index_offset].z = 0.f;
	vertex[index_offset].uv_x = uv_x;
	vertex[index_offset].uv_y = uv_y;
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
		MoonPrompt((char*)"图形顶点溢出");
		return MOON_FALSE;
	}

	if (moon_vertex_texture_index >= (unsigned int)MOON_VERTICES_MAX - 6)
	{
		MoonPrompt((char*)"纹理顶点溢出");
		return MOON_FALSE;
	}
	return MOON_TRUE;
}

//此函数仅作为辅助函数
static void MoonDrawAreaTemp(unsigned int message_type, unsigned int index, MOON_MESSAGE_ALL* message, MOON_IMAGE* image_old, MOON_IMAGE* image_resource_old)
{
	if ((index == message->message_index - 1)
		|| message->message[index + 1].message != message_type
		|| message->message[index + 1].metadata.draw.image_goal != image_old
		|| message->message[index + 1].metadata.draw.image.image_resources != image_resource_old
		|| message->message[index + 1].metadata.draw.color != message->message[index].metadata.draw.color)
	{
		MoonCoreDrawArea(moon_vertex_texture, moon_vertex_texture_index, &message->message[index].metadata);
		moon_vertex_texture_index = 0;
	}
}

#pragma once
#include"Moon.h"

/*
注意!
这个文件中的函数您不应该使用!

*/

typedef struct MOON_MESSAGE_SPECIFIC
{
	unsigned int message;				//消息类型
	MOON_METADATA metadata;				//数据元
}MOON_MESSAGE_SPECIFIC;

typedef struct
{
	MOON_MESSAGE_SPECIFIC* message;
	unsigned int message_index;
}MOON_MESSAGE_ALL;

typedef struct MOON_ENGINECORE
{
	_Bool dead;					//项目状态
	_Bool thread_message_type_draw;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息
	_Bool thread_message_type_logic;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息
	//_Bool thread_message_type_attr;//消息队列状态 MOON_FALSE为可用 MOON_TRUE为不可发送消息	
	_Bool gamepowermode;		//记录高性能模式旧模式
	char power;					//高性能模式
	int focus;					//焦点
	int(*Logic)(struct MOON_PROJECTGOD*);				//多线程逻辑函数
	int(*Drawing)(struct MOON_PROJECTGOD*);				//主线程绘图函数
	int(*Attr)(struct MOON_PROJECTGOD*);				//属性函数

	MOON_MESSAGE_ALL message_draw;//绘制消息队列
	MOON_MESSAGE_ALL message_logic;//逻辑消息队列
	//MOON_MESSAGE_ALL message_attr;//属性消息队列

	MOON_TIMELOAD timeload;		//计时器
}MOON_ENGINECORE;

typedef struct
{
	float x, y, z, uv_x, uv_y;
}MOON_TEXTURE_VECTER;		//用于纹理顶点

typedef void* MOON_ALLOC;
typedef struct
{
	MOON_ALLOC* alloc;
	unsigned int index;
}MOON_ALLOC_REGISTRY;		//用于内部队列

/*
* 函數 MoonWindow
* 作用 創建窗口
* 使用方法
* MOON_HWND* hwnd = MoonWindow(L"MyGame", 100, 100, 800, 600);
*/
_declspec(dllexport) extern MOON_HWND* MoonWindow(const char* name, int window_coord_x, int window_coord_y, int window_width, int window_height);					//创建窗口

/*
* 函數 MoonPrompt
* 作用 内部提示
* 使用方法
* MoonPrompt(text);
*/
_declspec(dllexport) extern void MoonPrompt(char* text);

/*
* 函數 MoonProjectError
* 作用 錯誤處理函數
* 使用方法
* MoonProjectError(NULL, 1, "Something went wrong!");
*/
_declspec(dllexport) extern int MoonProjectError(void* alpha, int degree, char* text);	//错误处理

/*
* 注意!這個函數對你的代碼沒有任何作用!僅僅是引擎内部初始化使用的,你可以使用MoonProjectFunctionSwitch來獲得更好的效果
* 函數 MoonProjectPause
* 作用 暫停/恢復項目運行
* 使用方法
* MoonProjectPause(1, &project->Logic, NULL, NULL);
*/
_declspec(dllexport) extern int MoonProjectPause(int mode, int (**function_1)(MOON_PROJECTGOD*), int (*function_2)(MOON_PROJECTGOD*), int (*function_3)(MOON_PROJECTGOD*));		//暂停函数

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonUtilityLoad
* 作用 初始化所有内部工具函數
* 使用方法
* MoonUtilityLoad(project);
*/
_declspec(dllexport) extern void MoonUtilityLoad(MOON_PROJECTGOD* project);

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonUtilityOver
* 作用 清理所有内部资源
* 使用方法
* MoonUtilityOver();
*/
_declspec(dllexport) extern void MoonUtilityOver();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonAlloc_Registry
* 作用 内部注册表名单+1
* 使用方法
* MoonAlloc_Registry();
*/
_declspec(dllexport) extern _Bool MoonAlloc_Registry();

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonDrawLoad
* 作用 初始化所有内部绘图函數
* 使用方法
* MoonDrawLoad(project);
*/
_declspec(dllexport) extern void MoonDrawLoad(MOON_PROJECTGOD* project);

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonDrawOver
* 作用 回收绘图模块的资源
* 使用方法
* MoonDrawOver();
*/
_declspec(dllexport) extern void MoonDrawOver();

/*
* 函數 MoonProjectFunctionSwitch
* 作用 切換項目中的函數指針
*		通过第二个参数,指定需要更换的模块
*		参数为
			MOON_MODULE_DRAW
			MOON_MODULE_LOGIC
* 使用方法
* MoonProjectFunctionSwitch(project, MOON_MODULE_DRAW, NewDrawingFunction);
*/
_declspec(dllexport) extern void MoonProjectFunctionSwitch(char module, int (*function_2)(MOON_PROJECTGOD*));//函数切换

/*
* 注意!這個函數對你的代碼可能沒有任何作用!
* 函數 MoonProjectGetMessage
* 作用 初始化所有内部工具函數
* 使用方法
* MoonProjectGetMessage(message, handle);
*/
_declspec(dllexport) extern int MoonProjectGetMessage(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type, void(*Handle)(MOON_PROJECTGOD*, MOON_MESSAGE_ALL*, _Bool*));	//获取消息
_declspec(dllexport) extern void MoonDrawMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type);	//处理绘制线程消息
_declspec(dllexport) extern void MoonlogicMessageHandle(MOON_PROJECTGOD* project, MOON_MESSAGE_ALL* message, _Bool* type);	//处理逻辑线程消息

//-------------------------------------------------------------------------------------------绘制函数--------------------------------------------------------------------------------//

_declspec(dllexport) extern void MoonCoreDrawArea(MOON_TEXTURE_VECTER* vertexs, unsigned int vertex_number, MOON_METADATA* metadata);					//画板
_declspec(dllexport) extern int MoonCoreGraphic(MOON_POINT3D* vertexs, unsigned int vertex_number, MOON_METADATA* metadata, unsigned int message_type);	//绘制图案
_declspec(dllexport) extern void MoonCoreFont(MOON_METADATA* metadata);																					//渲染默认字体

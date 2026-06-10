#include"MoonType.h"

//------------------------------------基础工具函数-----------------------------------------------//

/*
* 函數 MoonHash
* 作用 求字符串的Hash值
* 使用方法
* char arr[12] = "Hello_World";
* int alpha = Moonhash(arr)
*/
_declspec(dllexport) extern unsigned int MoonHash(char* text);

/*
* 函數 DegRad
* 作用 求弧度值
* 使用方法
* float alpha = cos(DegRad(45));
*/
#define MoonDegRad(phi) (MOON_Pi * (phi) * 1.f / 180.f)//角度转弧度

/*
* 函數 MoonKeyState
* 作用 獲取硬件狀態
* 使用方法
* _Bool alpha = MoonKeyState(MOON_KEY_SPACE);
*/
_declspec(dllexport) extern _Bool MoonKeyState(unsigned int Key);//获取按键的值

/*
* 函數 MoonKeyReal
* 作用 獲取硬件狀態
*     即时检测
* 使用方法
* _Bool alpha = MoonKeyReal(MOON_KEY_MOUSE_LEFT);
*/
_declspec(dllexport) extern _Bool MoonKeyReal(unsigned int Key);//获取按键的值

/*
* 函數 Lerp
* 作用 線性插值
* 使用方法
* float alpha = Lerp(0, 1, .5f);
*/
#define MoonLerp(alpha, beta, t) ((1.f - (t)) * (alpha) + (t) * (beta))				//线性插值

/*
* 函數 Moonmax
* 作用 限制範圍
* 使用方法
* int alpha = Moonmax(0, 100);
*/
#define MoonMax(a,b) (((a) > (b)) ? (a) : (b))

/*
* 函數 Moonmin
* 作用 限制範圍
* 使用方法
* int alpha = Moonmin(0, 100);
*/
#define MoonMin(a,b) (((a) < (b)) ? (a) : (b))

/*
* 函數 RANGE
* 作用 限制範圍
* 使用方法
* int alpha = MoonRange(beta, 0, 100);
*/
#define MoonRange(alpha, alpha_min, alpha_max) (MoonMin(MoonMax(alpha, alpha_min), alpha_max))//限制范围

/*
* 函數 MoonSetMouseCoord
* 作用 設置窗口输入中光标的状态
* 使用方法
* MoonSetMouseCoord(x, y);
*/
_declspec(dllexport) extern void MoonSetMouse(MOON_CURSOR_MODE mode);//设置鼠标位置

/*
* 函數 Random
* 作用 獲取隨機數
* 使用方法
* int alpha = Random(0, 100);
*/
#define MoonRandom(A, B) (rand() % ((B) - (A)) + (A))//随机数获取

/*
* 函數 MoonMusic
* 作用 播放音樂
* 使用方法
* MoonMusic("music.mp3");
*/
_declspec(dllexport) extern void MoonMusic(const char* File);//播放音乐

/*
* 函數 MoonTriangleDetection
* 作用 檢測點與三角形的位置關係
* 使用方法
* MOON_POINT2D point[4] =
* {
*	{0,0},
*	{10,0},
*	{0,10},
*	{5,50}
* };
* _Bool = MoonTriangleDetection(point[0], point[1], point[2], point[3]);
*/
_declspec(dllexport) extern int MoonTriangleDetection(MOON_POINT2D a, MOON_POINT2D b, MOON_POINT2D c, MOON_POINT2D p);	//三角形碰撞检测

/*
* 函數 MoonString
* 作用 字符串拼接
* 使用方法
* MoonString(str)
*/
#define MoonString(str)	""#str

/*
* 函數 MoonRGBA
* 作用 写入颜色值
* 使用方法
* MoonRGBA(r,g,b,a)
*/
#define MoonRGBA(r,g,b,a) ((r)|((g) << 8)|((b) << 16)|((a) << 24))

//------------------------------------定时函数--------------------------------------------------//


/*
* 函數 MoonTimeLoadInit
* 作用 初始化定時器
* 使用方法
* TIMELOAD load;
* MoonTimeLoadInit(&load, 100);
*/
_declspec(dllexport) extern void MoonTimeLoadInit(MOON_TIMELOAD* Timeload, int load);		//初始化定时器

/*
* 函數 MoonTimeLoad
* 作用 運行定時器
* 使用方法
* MoonTimeLoad(&load, 1);
*/
_declspec(dllexport) extern int MoonTimeLoad(MOON_TIMELOAD* Timeload, int mode);//运行定时器

/*
* 函數 MoonSleep
* 作用 暫停
* 使用方法
* MoonSleep(1);
*/
_declspec(dllexport) extern int MoonSleep(int timeload);					//暂停

//------------------------------------实体函数--------------------------------------------------//


/*
* 函數 MoonFindEntity
* 作用 尋找實體
* 使用方法
* int* entity = (int*)MoonFindEntity(projectgod, (char*)nameid)
*/
_declspec(dllexport) extern void* MoonFindEntity(MOON_PROJECTGOD* project, char* nameid);	//寻找实体

/*
* 函數 HashFindEntity
* 作用 hash寻找实体
* 使用方法
* HashFindEntity(project, "ProjectBitmap", int, engineback);
*/
#define MoonHashFindEntity(projectgod, nameid, type, entity) type* entity = (type*)MoonFindEntity(projectgod, (char*)nameid)//hash寻找实体

/*
* 函數 MoonCreateEntityIndex
* 作用 注册实体
* 使用方法
* static TIMELOAD logictps;
* MoonCreateEntityIndex(project, &logictps, "LogicTps", sizeof(TIMELOAD));
*/
_declspec(dllexport) extern int MoonCreateEntityIndex(MOON_PROJECTGOD* project, void* arrentity, char* nameid, size_t size_len, char* type_name);//注册实体

//------------------------------------双缓冲函数------------------------------------------------//


/*
* 函數 MoonCreateImage
* 作用 创建双缓冲绘图绘图区
* 使用方法
* IMAGE buffer;
* MoonCreateImage(&buffer, project->window_width, project->window_height);
*/
_declspec(dllexport) extern void MoonImageCreate(MOON_IMAGE* image, int bmpwidth, int bmpheight);	//创建双缓冲绘图绘图区

/*
* 函數 MoonDeletImage
* 作用 删除双缓冲绘图绘图区/紋理
* 使用方法
* MoonDeletImage(bitmap);
*/
_declspec(dllexport) extern void MoonImageDelete(MOON_IMAGE* image);//删除双缓冲绘图绘图区

//------------------------------------多线程函数------------------------------------------------//


/*
* 函數 创建多线程函数
* 作用 CREATETHREADFUNCTION
* 使用方法
* CREATETHREADFUNCTION(Thread);
*/
#define MOON_CREATETHREADFUNCTION(NAME)       MOON_THREAD NAME(void* lparam)				//创建多线程函数

/*
* 函數 CREATETHREAD
* 作用 创建并运行多线程函数
* 使用方法
* CREATETHREAD(Thread, project);
*/
#define MOON_CREATETHREAD(fuction, name, resource)   SDL_CreateThread(fuction, name, (void*)resource);//创建并运行多线程函数

/*
* 函數 GETTHREADRESOURCE
* 作用 获取多綫程函數外部导入的资源
* 使用方法
* static CREATETHREADFUNCTION(Thread)
* {
*	GETTHREADRESOURCE(MOON_PROJECTGOD*, project);
*	return 1;
* }
*/
#define MOON_GETTHREADRESOURCE(type, resource) type resource = (type)lparam;	//获取外部导入的资源

//------------------------------------字符函数------------------------------------------------//


/*
* 函數 MoonCharToWchar
* 作用 字符转换
* 使用方法
* wchar_t* text1;
* char* text2 = "Hello";
* MoonCharToWchar(text1, text2, 5);
*/
_declspec(dllexport) extern int MoonCharToWchar(wchar_t* text1, char* text2, int len);//字符转换

/*
* 函數
* 作用 字符转换
* 使用方法
* char* text1;
* wchar_t* text2 = L"Hello";
* MoonCharToWchar(text1, text2, 5);
*/
_declspec(dllexport) extern int MoonWcharToChar(char* text1, wchar_t* text2, int len);//字符转换

//------------------------------------按钮控件------------------------------------------------//

/*
* 函數 MoonButtonInit
* 作用 初始化按鈕控件
* 使用方法
* MOONBUTTON btn;
* MoonButtonInit(&btn, 10, 10, 100, 50);
*/
_declspec(dllexport) extern int MoonButtonInit(MOON_BUTTON* button, int x, int y, int width, int height);		//初始化按钮

/*
* 函數 MoonButtonDetection
* 作用 檢測按鈕觸發狀態
* 使用方法
* if(MoonButtonDetection(button, x, y, context) { ... }
*/
_declspec(dllexport) extern int MoonButtonDetection(MOON_BUTTON* button, int x, int y, void* context);

/*
* 函數 MoonButtonSetTriggerMode
* 作用 更改按鈕觸發方式
* 使用方法
* MoonButtonSetTriggerMode(project, "MyButton", MOON_BUTTONPRESS);
*/
_declspec(dllexport) extern int MoonButtonSetTriggerMode(MOON_PROJECTGOD* project, MOON_BUTTON* button, unsigned int key);//更改触发方式

/*
* 函數 MOONBUTTONCREATE
* 作用 便捷注冊按鈕
* 使用方法
* static MOONBUTTON button
* MOONBUTTONCREATE(project, name, x, y, w, h, 0, 0, 0);
*/
#define MOON_BUTTON_CREATE(project, name, button, x, y, width, height, Press, PressL, Hover)  \
{	\
	MoonButtonInit(&button,(x), (y), (width), (height));                                        \
	MoonButtonSetTriggerMode(project,&button,MOON_KEY_MOUSE_LEFT);								\
	button.ButtonModeHover = (Hover);                                                           \
	button.ButtonModePress = (Press);                                                           \
	button.ButtonModePressL = (PressL);                                                         \
	MoonCreateEntityIndex(project, &button, (char*)name, sizeof(MOON_BUTTON), "MOON_BUTTON");	\
}
/*
* 函數 MoonFileLoad_TEXT
* 作用 加载文本文件
* 使用方法
*	MoonFileLoad_TEXT("a.txt", text, strlen(text));
*/
_declspec(dllexport) extern _Bool MoonFileLoad_TEXT(char* file_name, char* text, unsigned int text_size);
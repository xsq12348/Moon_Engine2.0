#pragma once
#include"Moon.h"
#include"Type.h"

#define NewKeyState(vKey) GetAsyncKeyState(vKey)

/*
* 項目創建時間 : 
* 日志
* 
*/

/*
* 函數 GameDrawAll
* 作用 主繪製函數
*/
extern MOON_PROJECTMODULE(GameDrawAll);

/*
* 函數 GameDrawLoad
* 作用 主繪製函數加載
*/
extern MOON_PROJECTMODULE(GameDrawLoad);

/*
* 函數 GameLogicAll
* 作用 主邏輯函數
*/
extern MOON_PROJECTMODULE(GameLogicAll);

/*
* 函數 GameLogicLoad
* 作用 主邏輯函數加載
*/
extern MOON_PROJECTMODULE(GameLogicLoad);

/*
* 函數 _Resource
* 作用 主加載資源函數
*/
extern MOON_PROJECTSETTING(_Resource);

/*
* 函數 SettingAll
* 作用 游戲開始前設置
*/
extern MOON_PROJECTSETTING(SettingAll);

/*
* 函數 SettingOver
* 作用 游戲結束處理
*/
extern MOON_PROJECTSETTING(SettingOver);
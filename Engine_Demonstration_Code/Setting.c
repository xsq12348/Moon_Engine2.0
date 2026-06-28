#include "Project_All.h"

extern MOON_PROJECTSETTING(SettingAll)
{
	{
		MoonHashFindEntity("LogicTps", MOON_TIMELOAD, logicTps);
		MoonTimeLoadInit(logicTps, 1000 / 60); // 60 FPS逻辑帧
	}

	{
		GameDrawLoad();
		GameLogicLoad();
	}
}

extern MOON_PROJECTSETTING(SettingOver)
{
	// 可添加清理代碼（如果需要）
}
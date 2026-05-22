#include "Project_All.h"

extern MOON_PROJECTSETTING(SettingAll)
{
	{
		MoonHashFindEntity(project, "LogicTps", MOON_TIMELOAD, logicTps);
		MoonTimeLoadInit(logicTps, 1000.f / 60.f); // 60 FPS逻辑帧
	}
}

extern MOON_PROJECTSETTING(SettingOver)
{
	// 可添加清理代碼（如果需要）
}
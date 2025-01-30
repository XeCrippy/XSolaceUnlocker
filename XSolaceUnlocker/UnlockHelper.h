#pragma once
#include "stdafx.h"

namespace AchievementUnlocker {
	class achievementUnlocker {
	public:
		static void LoadPlugin();
	private:
		static uint32_t GetIdForIndex(uint32_t index);
		static void UnlockAchievements();
		static void UnlockAvatarAwards();
	};
}
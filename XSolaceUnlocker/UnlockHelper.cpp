#include "stdafx.h"
#include "UnlockHelper.h"

namespace AchievementUnlocker {

	const wchar_t* buttonLabels[] = { L"Continue" };
	const wchar_t* Title = L"XSolace Achievement & Award Unlocker";

	const wchar_t* WelcomePage() {
		std::wstring newLine = L"\r\n";
		std::wstring welcome;
		welcome += L"Instructions:";
		welcome += newLine;
		welcome += L"Load the game you want and press :";
		welcome += newLine;
		welcome += L"Unlock Achievements : Dpad_Left+X";
		welcome += newLine;
		welcome += L"Unlock Avatar Awards : Dpad_Left+Y";
		welcome += newLine + newLine;
		welcome += L"Made by XeCrippy";
		return welcome.c_str();
	}

	void ShowWelcomeMsg() {
		Utilities::Xam::ShowMessageBox(
			Title,
			WelcomePage(),
			buttonLabels,
			ARRAYSIZE(buttonLabels),
			nullptr
		);
	}

	uint32_t achievementUnlocker::GetAchievementIdForIndex(uint32_t index) {
		return index + 1;
	}

	uint32_t achievementUnlocker::GetAwardIdForIndex(uint32_t index) {
		return index + 1;
	}

	void achievementUnlocker::UnlockAchievements() {
		const uint32_t numAchievements = 250; // this value can be experimented with 
		XUSER_ACHIEVEMENT achievements[numAchievements] = {};

		for (uint32_t i = 0; i < numAchievements; ++i) {
			achievements[i].dwAchievementId = GetAchievementIdForIndex(i); // creates array of XUSER_ACHIEVEMENTS, increasing the dwAchievementId each iteration so {1,2,3,4......250}
		}

		//XOVERLAPPED overlapped = {}; // for asynchronous operation
		PXOVERLAPPED pOverlapped = nullptr; // use &overlapped for asynchronous operation 

		uint32_t result = XUserWriteAchievements(numAchievements, achievements, pOverlapped);

		if (result != ERROR_SUCCESS) {
			Utilities::Xam::XNotify("Failed to unlock achievements");
		}
	}

	void achievementUnlocker::UnlockAvatarAwards() {
		const uint32_t numAwards = 50;
		XUSER_AVATARASSET avatarAwards[numAwards] = {};

		for (uint32_t i = 0; i < numAwards; ++i) {
			avatarAwards[i].dwAwardId = GetAwardIdForIndex(i);
		}

		XOVERLAPPED overlapped = {};
		PXOVERLAPPED pOverlapped = nullptr;

		uint32_t result = XUserAwardAvatarAssets(numAwards, avatarAwards, pOverlapped);

		if (result != ERROR_SUCCESS) {
			Utilities::Xam::XNotify("Failed to unlock avatar awards. Check if the game has any");
		}
	}

	void achievementUnlocker::LoadPlugin() {
		XINPUT_STATE state = { 0 };
		ZeroMemory(&state, sizeof(state));

		ShowWelcomeMsg();

		while (true) {

			bool hasToggled = false;

			if (XInputGetState(0, &state) == ERROR_SUCCESS) {

				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_X) {
					Utilities::Xam::PulseController();
					UnlockAchievements();
					hasToggled = true;
				}
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
					Utilities::Xam::PulseController();
					UnlockAvatarAwards();
					hasToggled = true;
				}
			}
			if (hasToggled) Sleep(40);
		}

	}
}

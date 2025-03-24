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
		welcome += L"Sign into the account you want to use";
		welcome += newLine;
		welcome += L"Load the game you want and press :";
		welcome += newLine;
		welcome += L"Unlock Achievements : Dpad_Left+X";
		welcome += newLine;
		welcome += L"Unlock Single Achievement : Dpad_Left+B";
		welcome += newLine;
		welcome += L"Unlock Avatar Awards : Dpad_Left+Y";
		welcome += newLine;
		welcome += L"Unlock Gamer Pics : Dpad_Left+A";
		welcome += newLine + newLine;
		welcome += L"Reboot : LB+RB+A";
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

	uint32_t achievementUnlocker::GetIdForIndex(uint32_t index) {
		return index + 1;
	}

	void achievementUnlocker::UnlockSingleAchievement() {
		XINPUT_STATE state = {};
		const uint32_t numAchievements = 1;
		XUSER_ACHIEVEMENT achievements[numAchievements] = {};

		unsigned long hr = 0;
		HRESULT hre = 0;
		wchar_t r1[512];
		XOVERLAPPED overlapped = {};
		PXOVERLAPPED pOverlapped = &overlapped;

		ZeroMemory(&state, sizeof(XINPUT_STATE));

		XInputGetState(0, &state);

		hre = XShowKeyboardUI(0, VKBD_DEFAULT, L"", L"Individual Achievement Unlocker", L"Enter achievement Id", r1, 512, pOverlapped);

		while (!XHasOverlappedIoCompleted(pOverlapped)) Sleep(30);

		if (hre == ERROR_IO_PENDING)  hr = _wtoi(r1); 

		achievements->dwUserIndex = 0;
		achievements->dwAchievementId = (DWORD)hr;

		XUserWriteAchievements(numAchievements, achievements, pOverlapped);

		while (!XHasOverlappedIoCompleted(pOverlapped)) Sleep(30);

		uint32_t overlappedResult = XGetOverlappedResult(pOverlapped, nullptr, TRUE);
		if (overlappedResult != ERROR_SUCCESS)
			Utilities::Xam::XNotify("Failed to unlock achievements!");
	}

	void achievementUnlocker::UnlockAchievements() {
		const uint32_t numAchievements = 500;
		XUSER_ACHIEVEMENT achievements[numAchievements] = {};

		for (uint32_t i = 0; i < numAchievements; ++i) {
			achievements[i].dwUserIndex = 0;
			achievements[i].dwAchievementId = GetIdForIndex(i);
		}

		XOVERLAPPED overlapped = {};
		PXOVERLAPPED pOverlapped = &overlapped;

		XUserWriteAchievements(numAchievements, achievements, pOverlapped);

		while (!XHasOverlappedIoCompleted(pOverlapped))
			Sleep(30);

		uint32_t overlappedResult = XGetOverlappedResult(pOverlapped, nullptr, TRUE);
		if (overlappedResult != ERROR_SUCCESS)
			Utilities::Xam::XNotify("Failed to unlock achievements!");
	}

	void achievementUnlocker::UnlockAvatarAwards() {
		const uint32_t numAwards = 30;
		XUSER_AVATARASSET avatarAwards[numAwards] = {};

		for (uint32_t i = 0; i < numAwards; ++i) {
			avatarAwards[i].dwAwardId = GetIdForIndex(i);
		}

		XOVERLAPPED overlapped = {}; 
		PXOVERLAPPED pOverlapped = &overlapped; 

		XUserAwardAvatarAssets(numAwards, avatarAwards, pOverlapped);

		uint32_t overlappedResult = XGetOverlappedResult(pOverlapped, nullptr, TRUE);
		if (overlappedResult != ERROR_SUCCESS)
			Utilities::Xam::XNotify("Failed to unlock avatar awards. Check if the game has them!");
	}

	void achievementUnlocker::UnlockGamerPics() {
		uint32_t userIndex = 0;
		const uint32_t numPics = 50;
		uint32_t gamerPicId = 1;
		uint32_t result = 0;

		XOVERLAPPED overlapped = {};
		PXOVERLAPPED pOverlapped = &overlapped;

		for (uint32_t i = 0; i < numPics; ++i) {
			result = XUserAwardGamerPicture(userIndex, i, 0, pOverlapped);
			while (!XHasOverlappedIoCompleted(pOverlapped)) Sleep(10);
		}

		uint32_t overlappedResult = XGetOverlappedResult(pOverlapped, nullptr, TRUE);
		if (overlappedResult != ERROR_SUCCESS)
			Utilities::Xam::XNotify("Failed to unlock gamer pics. Check if the game has them!");
	}

	void achievementUnlocker::LoadPlugin() {
		XINPUT_STATE state = { 0 };
		ZeroMemory(&state, sizeof(state));
		
		ShowWelcomeMsg();

		while (true) {

			bool hasToggled = false;
			uint32_t tid = Utilities::Xam::GetCurrentTitleId();

			if (XInputGetState(0, &state) == ERROR_SUCCESS) {

				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_X && tid != Utilities::DASHBOARD) {
					Utilities::Xam::PulseController();
					UnlockAchievements();
					hasToggled = true;
				}
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
					Utilities::Xam::PulseController();
					UnlockAvatarAwards();
					hasToggled = true;
				}
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
					Utilities::Xam::PulseController();
					UnlockSingleAchievement();
					hasToggled = true;
				}
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
					Utilities::Xam::PulseController();
					UnlockGamerPics();
					hasToggled = true;
				}
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER && state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER && state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
					Utilities::Xam::Reboot();
					hasToggled = true;
				}
			}
			if (hasToggled) Sleep(40);
		}

	}
}

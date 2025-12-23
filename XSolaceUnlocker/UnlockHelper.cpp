#include "stdafx.h"
#include "UnlockHelper.h"
#define __isync() __emit(0x4C00012C)

namespace AchievementUnlocker {

	const wchar_t* buttonLabels[] = { L"Continue" };
	const wchar_t* Title = L"XSolace Achievement & Award Unlocker";
	const std::string& notify_on = "Leaderboard Hook : Enabled";
	const std::string& notify_off = "Leaderboard Hook : Disabled";
	bool lb_hooked = false;

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
		welcome += newLine;
		welcome += L"Leaderboard Patcher : Dpad_Right+A";
		welcome += newLine + newLine;
		welcome += L"Reboot Console : LB+RB+A";
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

	void PatchInJump(uint32_t* addr, void* dest, BOOL Linked)
	{
		if ((uint32_t)dest & 0x8000)
			addr[0] = 0x3D600000 + ((((uint32_t)dest >> 16) & 0xFFFF) + 1);
		else
			addr[0] = 0x3D600000 + (((uint32_t)dest >> 16) & 0xFFFF);

		addr[1] = 0x396B0000 + ((uint32_t)dest & 0xFFFF);
		addr[2] = 0x7D6903A6; // mtctr r11
		addr[3] = Linked ? 0x4E800421 : 0x4E800420; // bctrl or bctr
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

		while (!XHasOverlappedIoCompleted(pOverlapped))
			Sleep(30);

		uint32_t overlappedResult = XGetOverlappedResult(pOverlapped, nullptr, TRUE);
		if (overlappedResult != ERROR_SUCCESS)
			Utilities::Xam::XNotify("Failed to unlock avatar awards. Check if the game has them!");
	}

	void achievementUnlocker::UnlockGamerPics() {
		uint32_t userIndex = 0;
		const uint32_t numPics = 15;
		uint32_t gamerPicId = 1;
		uint32_t result = 0;

		XOVERLAPPED overlapped = {};
		PXOVERLAPPED pOverlapped = &overlapped;

		for (uint32_t i = 0; i < numPics; ++i) {
			result = XUserAwardGamerPicture(userIndex, i, 0, pOverlapped);
			while (!XHasOverlappedIoCompleted(pOverlapped)) Sleep(10);
		}
	}

	void PatchSessionViews(uint32_t pViewsAddr, uint32_t dwNumViews, uint32_t targetPropertyId) {
		if (!pViewsAddr || dwNumViews == 0) return;

		const uint32_t VIEW_SIZE = 12;
		const uint32_t ENTRY_SIZE = 0x18;

		if (dwNumViews > 1024) return;

		for (uint32_t vi = 0; vi < dwNumViews; ++vi) { // Iterate through each available view. Usually will be more than 1.
			uint32_t viewAddr = pViewsAddr + vi * VIEW_SIZE;

			uint32_t numProps = *(uint32_t*)(viewAddr + 4);
			uint32_t pProps = *(uint32_t*)(viewAddr + 8);

			if (!pProps || numProps == 0) continue;
			if (numProps > 100000) break;

			for (uint32_t pi = 0; pi < numProps; ++pi) {
				uint32_t entry = pProps + pi * ENTRY_SIZE;

				uint32_t propId = *(uint32_t*)(entry + 0);
				uint8_t type = *(uint8_t*)(entry + 8);

				if (targetPropertyId != 0) {
					if (propId != targetPropertyId) continue;
				}
				else {
					if ((propId & 0xFF000000) != 0x20000000 && (propId & 0xFF000000) != 0x10000000) continue; // only checking for int32 and int64
				}

				if (type == 1) {
					int32_t v32 = 0x7FFFFFFF;
					*(int32_t*)(entry + 0x10) = v32;
				}
				else if (type == 2) {
					int64_t v64 = 0x7FFFFFFFFFFFFFFFLL;
					*(int64_t*)(entry + 0x10) = v64;
				}
				else if (type == 3) {
					float fv = (float)-1;
					*(float*)(entry + 0x10) = fv;
					//currently unused
				}
				else if (type == 4) {
					double dv = (double)-1;
					*(double*)(entry + 0x10) = dv;
					//currently unused
				}
				else {
					//BYTE, STRING, BINARY currently not handled
				}
			}
		}
	}

	__declspec(naked) void __return_function() {
		__asm {
			mr r31, r3
			mr r29, r4
			mr r27, r5
			mr r30, r6

			lis r12, 0x816a
			ori r12, r12, 0x2D0C // Retail Only. Devkit return address will be different. 
			mtctr r12
			bctr
		}
	}

	__declspec(naked) void __savegprlr() {
		__asm {
			stmw    r14, 0x50(r1)
			stmw    r3, 0x08(r1)
			blr
		}
	}

	__declspec(naked) void __restgprlr() {
		__asm {
			lmw     r3, 0x08(r1)
			lmw     r14, 0x50(r1)
			lwz     r0, 0xA4(r1)
			addi    r1, r1, 0xA0
			mtlr    r0
			bl __return_function
		}
	}

	__declspec(naked) void LeaderboardHook_Testing()
	{
		// Hook to XMessageIOStartRequestEx in xam.xex
		// Monitor r3 and r4 to check for XSessionWriteStats being processed
		// If so, patch the views to max the leaderboard stats
		__asm {
			cmpwi r3, 0xFB
			bne skip_patch

			lis   r0, 0x000B
			ori   r0, r0, 0x0025
			cmplw r0, r4
			bne   skip_patch

			stwu  r1, -0xA0(r1)
			mflr  r0
			stw   r0, 0xA4(r1)
			bl    __savegprlr

			lwz  r3, 0x14(r6) // *pViews
			lwz  r4, 0x10(r6) // dwNumViews   
			li   r5, 0        // targetPropertyId = 0 (all properties)              

			bl PatchSessionViews

			bl __restgprlr

			skip_patch :
			bl __return_function
		}
	}

	void SetLeaderboardHook() {
		uint32_t hookEntryAddr = (uint32_t)Utilities::Memory::ResolveFunction("xam.xex", 0x1FC) + 0xC; //0x816A2CFC;
		PatchInJump((uint32_t*)hookEntryAddr, (void*)LeaderboardHook_Testing, FALSE);
		__dcbst(0, LeaderboardHook_Testing);
		__isync();
	}

	void DisableLeaderboardHook() {
		//7C 7F 1B 78 7C 9D 23 78 7C BB 2B 78 7C DE 33 78
		uint32_t hookEntry = (uint32_t)Utilities::Memory::ResolveFunction("xam.xex", 0x1FC) + 0xC;
		uint32_t originalBytes[] = { 0x7C7F1B78, 0x7C9D2378, 0x7CBB2B78, 0x7CDE3378 };
		for (size_t i = 0; i < 4; ++i) {
			*((uint32_t*)(hookEntry + i * 4)) = originalBytes[i];
		}
		__dcbst(0, (void*)hookEntry);
		__isync();
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
				if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT && state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
					if (!lb_hooked) {
						Utilities::Xam::PulseController();
						SetLeaderboardHook();
						Utilities::Xam::XNotify(notify_on);
						lb_hooked = true;
					}
					else {
						Utilities::Xam::PulseController();
						DisableLeaderboardHook();
						Utilities::Xam::XNotify(notify_off);
						lb_hooked = false;
					}
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

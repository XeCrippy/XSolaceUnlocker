#include "stdafx.h"

struct STRING
{
    uint16_t Length;
    uint16_t MaxLength;
    char* Buffer;
};

// Imports from xboxkrnl.exe and xam.xex
extern "C"
{
    DWORD XamGetCurrentTitleId();

    bool MmIsAddressValid(void* pAddress);

    void HalReturnToFirmware(uint32_t powerDownMode);

    void RtlInitAnsiString(STRING* pDestinationString, const char* sourceString);

    HRESULT ObCreateSymbolicLink(STRING* pLinkName, STRING* pDevicePath);

}

namespace Utilities {
    typedef void (*XNOTIFYQUEUEUI)(XNOTIFYQUEUEUI_TYPE type, uint32_t userIndex, uint64_t areas, const wchar_t* displayText, void* pContextData);
    static XNOTIFYQUEUEUI XNotifyQueueUI = static_cast<XNOTIFYQUEUEUI>(Memory::ResolveFunction("xam.xex", 656));

    uint32_t Xam::ShowMessageBox(const wchar_t* title, const wchar_t* text, const wchar_t** buttonLabels, size_t numberOfButtons, uint32_t* pButtonPressedIndex, uint32_t messageBoxType, uint32_t focusedButtonIndex)
    {
        MESSAGEBOX_RESULT messageBoxResult = { 0 };
        XOVERLAPPED overlapped = { 0 };

        // Open the message box
        XShowMessageBoxUI(
            0,
            title,
            text,
            numberOfButtons,
            buttonLabels,
            focusedButtonIndex,
            messageBoxType,
            &messageBoxResult,
            &overlapped
        );

        // Wait until the message box closes
        while (!XHasOverlappedIoCompleted(&overlapped))
            Sleep(100);

        // Get how the message box was closed (success, canceled or internal error)
        uint32_t overlappedResult = XGetOverlappedResult(&overlapped, nullptr, TRUE);

        // If the message box was closed by pressing "A" on any of the buttons (so not by pressing "B" or the Xbox button)
        // and if the pressed button is request, write the pressed button at pButtonPressedIndex
        if (overlappedResult == ERROR_SUCCESS && pButtonPressedIndex != nullptr)
            *pButtonPressedIndex = messageBoxResult.dwButtonPressed;

        return overlappedResult;
    }

    uint32_t Xam::GetCurrentTitleId()
    {
        return XamGetCurrentTitleId();
    }

    void Xam::PulseController()
    {
        XINPUT_VIBRATION xvib;
        xvib.wLeftMotorSpeed = 50000;
        xvib.wRightMotorSpeed = 50000;

        XInputSetState(0, &xvib);

        Sleep(300);

        xvib.wLeftMotorSpeed = 0;
        xvib.wRightMotorSpeed = 0;

        XInputSetState(0, &xvib);
    }

    void Xam::XNotify(const std::string& text, XNOTIFYQUEUEUI_TYPE type)
    {
        XNotifyQueueUI(type, 0, XNOTIFY_SYSTEM, Formatter::ToWide(text).c_str(), nullptr);
    }
}
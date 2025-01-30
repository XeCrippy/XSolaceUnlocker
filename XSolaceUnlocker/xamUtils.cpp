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

    uint32_t Xam::ShowKeyboard(const wchar_t* title, const wchar_t* description, const wchar_t* defaultText, std::string& result, size_t maxLength, uint32_t keyboardType)
    {
        // maxLength is the amount of characters the keyboard will allow, realMaxLength needs to include the \0 to terminate the string
        size_t realMaxLength = maxLength + 1;
        XOVERLAPPED overlapped = {};

        // Create the buffers
        wchar_t* wideBuffer = new wchar_t[realMaxLength];
        char* buffer = new char[realMaxLength];

        // Zero the buffers
        ZeroMemory(wideBuffer, sizeof(wideBuffer));
        ZeroMemory(buffer, sizeof(buffer));

        // Open the keyboard
        XShowKeyboardUI(
            0,
            keyboardType,
            defaultText,
            title,
            description,
            wideBuffer,
            realMaxLength,
            &overlapped
        );

        // Wait until the keyboard closes
        while (!XHasOverlappedIoCompleted(&overlapped))
            Sleep(100);

        // Get how the keyboard was closed (success, canceled or internal error)
        uint32_t overlappedResult = XGetOverlappedResult(&overlapped, nullptr, TRUE);
        if (overlappedResult == ERROR_SUCCESS)
        {
            // Convert the wide string to a narrow string
            wcstombs_s(nullptr, buffer, realMaxLength, wideBuffer, realMaxLength * sizeof(wchar_t));

            // Populate the out string with the narrow string
            result = buffer;
        }

        // Cleanup
        delete[] wideBuffer;
        delete[] buffer;

        return overlappedResult;
    }

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

    void Xam::Reboot()
    {
        // Declared in xkelib
        const uint32_t rebootRoutine = 1;

        HalReturnToFirmware(rebootRoutine);
    }

    void Xam::XNotify(const std::string& text, XNOTIFYQUEUEUI_TYPE type)
    {
        XNotifyQueueUI(type, 0, XNOTIFY_SYSTEM, Formatter::ToWide(text).c_str(), nullptr);
    }
}
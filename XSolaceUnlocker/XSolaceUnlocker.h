#pragma once
#include "stdafx.h"


// Credit to Clement Dreptin
class PluginMain {

public:
    static void Start();
    static void Stop();

private:
    static bool s_Running;

    static uint32_t s_CurrentTitleId;
    static uint32_t Update(void*);

    static void InitNewTitle(uint32_t newTitleId);
};
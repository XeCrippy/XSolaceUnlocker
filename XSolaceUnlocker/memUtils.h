#pragma once
#include "stdafx.h"

namespace Utilities {
	class Memory {
	public:
		static void* ResolveFunction(const std::string& moduleName, uint32_t ordinal);

		static void Thread(PTHREAD_START_ROUTINE pStartAddress, void* pArgs = nullptr);

		static void ThreadEx(PTHREAD_START_ROUTINE pStartAddress, void* pArgs, uint32_t creationFlags);
	};
}

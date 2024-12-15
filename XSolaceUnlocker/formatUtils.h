#pragma once
#include "stdafx.h"

namespace Utilities {

	// credit clement dreptin

	class Formatter {

	public:
		static std::string Format(const char* format, ...);

		static std::wstring ToWide(const std::string& narrowString);
	};
}

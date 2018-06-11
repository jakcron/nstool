#pragma once
#include <nx/nacp.h>

namespace nx
{
	class ApplicationControlPropertyUtils
	{
	public:
		static bool validateSaveDataSizeMax(int64_t size, int64_t alignment);
		static bool validateSaveDataSize(int64_t size);
	};
}
#pragma once
#include <fnd/types.h>
#include <fnd/rsa.h>
#include <nn/hac/macro.h>

namespace nn
{
namespace hac
{
	namespace kc
	{
		enum KernelCapId
		{
			KC_INVALID = 0,
			KC_THREAD_INFO = 3,
			KC_ENABLE_SYSTEM_CALLS = 4,
			KC_MEMORY_MAP = 6,
			KC_IO_MEMORY_MAP = 7,
			KC_ENABLE_INTERUPTS = 11,
			KC_MISC_PARAMS = 13,
			KC_KERNEL_VERSION = 14,
			KC_HANDLE_TABLE_SIZE = 15,
			KC_MISC_FLAGS = 16
		};
	}
}
}
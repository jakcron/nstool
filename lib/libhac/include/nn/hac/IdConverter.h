#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	class IdConverter
	{
	public:
		static uint64_t convertToAocBaseId(uint64_t application_id);
		static uint64_t convertToDeltaId(uint64_t application_id);
		static uint64_t convertToPatchId(uint64_t application_id);

	private:
		static const uint64_t kAocBaseId = 0x1000;
		static const uint64_t kDeltaId = 0xc00;
		static const uint64_t kPatchId = 0x800;
	};
}
}
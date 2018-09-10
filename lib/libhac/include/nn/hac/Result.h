#pragma once
#include <fnd/types.h>

namespace nn
{
namespace hac
{
	class Result
	{
	public:
		Result();
		Result(uint32_t result);
		Result(uint32_t module_num, uint32_t desc, uint32_t sub_desc);

		void operator=(const Result& other);
		bool operator==(const Result& other) const;
		bool operator!=(const Result& other) const;

		bool isSuccess() const;
		bool isFailure() const;

		uint32_t getInnerValue() const;
		uint32_t getModuleNum() const;
		uint32_t getDescription() const;
		uint32_t getSubDescription() const;

	private:
		static const uint32_t kModuleNumBitWidth = 9;
		static const uint32_t kModuleNumBitPos = 0;
		static const uint32_t kDescriptionBitWidth = 13;
		static const uint32_t kDescriptionBitPos = 9;
		static const uint32_t kSubDescriptionBitWidth = 10;
		static const uint32_t kSubDescriptionBitPos = 22;
		inline uint32_t bitWidthToMask(uint32_t bit_width) const { return _BIT(bit_width) - 1; }

		uint32_t mResult;
	};
}
}
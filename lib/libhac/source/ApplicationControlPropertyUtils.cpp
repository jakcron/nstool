#include <nx/ApplicationControlPropertyUtils.h>

bool nx::ApplicationControlPropertyUtils::validateSaveDataSizeMax(int64_t size, int64_t alignment)
{
	return (alignment != 0 && (size & (alignment - 1)) != 0) == false;
}

bool nx::ApplicationControlPropertyUtils::validateSaveDataSize(int64_t size)
{
	return (size & 0x3fff) == 0;
}

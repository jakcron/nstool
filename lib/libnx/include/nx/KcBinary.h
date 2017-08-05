#pragma once
#include <string>
#include <vector>
#include <fnd/MemoryBlob.h>
#include <fnd/List.h>
#include <fnd/ISerialiseableBinary.h>
#include <nx/ThreadInfoHandler.h>
#include <nx/SystemCallHandler.h>
#include <nx/MemoryMappingHandler.h>
#include <nx/InteruptHandler.h>
#include <nx/MiscParamsHandler.h>
#include <nx/KernelVersionHandler.h>
#include <nx/HandleTableSizeHandler.h>
#include <nx/MiscFlagsHandler.h>

namespace nx
{
	class KcBinary :
		public fnd::ISerialiseableBinary
	{
	public:
		KcBinary();
		KcBinary(const KcBinary& other);
		KcBinary(const u8* bytes, size_t len);

		bool operator==(const KcBinary& other) const;
		bool operator!=(const KcBinary& other) const;
		void operator=(const KcBinary& other);

		// to be used after export
		const u8* getBytes() const;
		size_t getSize() const;

		// export/import binary
		void exportBinary();
		void importBinary(const u8* bytes, size_t len);

		// variables (consider further abstraction?)
		void clear();
		const ThreadInfoHandler& getThreadInfo() const;
		ThreadInfoHandler& getThreadInfo();

		const SystemCallHandler& getSystemCalls() const;
		SystemCallHandler& getSystemCalls();

		const MemoryMappingHandler& getMemoryMaps() const;
		MemoryMappingHandler& getMemoryMaps();

		const InteruptHandler& getInterupts() const;
		InteruptHandler& getInterupts();

		const MiscParamsHandler& getMiscParams() const;
		MiscParamsHandler& getMiscParams();

		const KernelVersionHandler& getKernelVersion() const;
		KernelVersionHandler& getKernelVersion();

		const HandleTableSizeHandler& getHandleTableSize() const;
		HandleTableSizeHandler& getHandleTableSize();

		const MiscFlagsHandler& getMiscFlags() const;
		MiscFlagsHandler& getMiscFlags();

	private:
		const std::string kModuleName = "KC_BINARY";

		// raw binary
		fnd::MemoryBlob mBinaryBlob;

		// variables
		ThreadInfoHandler mThreadInfo;
		SystemCallHandler mSystemCalls;
		MemoryMappingHandler mMemoryMap;
		InteruptHandler mInterupts;
		MiscParamsHandler mMiscParams;
		KernelVersionHandler mKernelVersion;
		HandleTableSizeHandler mHandleTableSize;
		MiscFlagsHandler mMiscFlags;
		

		void clearVariables();
		bool isEqual(const KcBinary& other) const;
		void copyFrom(const KcBinary& other);
	};
}


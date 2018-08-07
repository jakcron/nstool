#pragma once
#include <string>
#include <vector>
#include <fnd/ISerialisable.h>
#include <fnd/List.h>
#include <nn/hac/ThreadInfoHandler.h>
#include <nn/hac/SystemCallHandler.h>
#include <nn/hac/MemoryMappingHandler.h>
#include <nn/hac/InteruptHandler.h>
#include <nn/hac/MiscParamsHandler.h>
#include <nn/hac/KernelVersionHandler.h>
#include <nn/hac/HandleTableSizeHandler.h>
#include <nn/hac/MiscFlagsHandler.h>

namespace nn
{
namespace hac
{
	class KernelCapabilityBinary :
		public fnd::ISerialisable
	{
	public:
		KernelCapabilityBinary();
		KernelCapabilityBinary(const KernelCapabilityBinary& other);

		void operator=(const KernelCapabilityBinary& other);
		bool operator==(const KernelCapabilityBinary& other) const;
		bool operator!=(const KernelCapabilityBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		virtual const fnd::Vec<byte_t>& getBytes() const;

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
		fnd::Vec<byte_t> mRawBinary;

		// variables
		ThreadInfoHandler mThreadInfo;
		SystemCallHandler mSystemCalls;
		MemoryMappingHandler mMemoryMap;
		InteruptHandler mInterupts;
		MiscParamsHandler mMiscParams;
		KernelVersionHandler mKernelVersion;
		HandleTableSizeHandler mHandleTableSize;
		MiscFlagsHandler mMiscFlags;
	};
}
}
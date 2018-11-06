#pragma once
#include <string>
#include <fnd/List.h>
#include <fnd/IByteModel.h>
#include <nn/hac/define/kip.h>
#include <nn/hac/KernelCapabilityControl.h>

namespace nn
{
namespace hac
{
	class KernelInitialProcessHeader :
		public fnd::IByteModel
	{
	public:
		struct sLayout
		{
			uint32_t offset;
			uint32_t size;

			void operator=(const sLayout& other)
			{
				offset = other.offset;
				size = other.size;
			}

			bool operator==(const sLayout& other) const
			{
				return (offset == other.offset) \
					&& (size == other.size);
			}

			bool operator!=(const sLayout& other) const
			{
				return !(*this == other);
			}
		};

		struct sCodeSegment
		{
			sLayout file_layout;
			sLayout memory_layout;
			bool is_compressed;

			void operator=(const sCodeSegment& other)
			{
				file_layout = other.file_layout;
				memory_layout = other.memory_layout;
				is_compressed = other.is_compressed;
			}

			bool operator==(const sCodeSegment& other) const
			{
				return (file_layout == other.file_layout) \
					&& (memory_layout == other.memory_layout) \
					&& (is_compressed == other.is_compressed);
			}

			bool operator!=(const sCodeSegment& other) const
			{
				return !(*this == other);
			}
		};

		KernelInitialProcessHeader();
		KernelInitialProcessHeader(const KernelInitialProcessHeader& other);

		void operator=(const KernelInitialProcessHeader& other);
		bool operator==(const KernelInitialProcessHeader& other) const;
		bool operator!=(const KernelInitialProcessHeader& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* data, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		const std::string& getName() const;
		void setName(const std::string& name);

		uint64_t getTitleId() const;
		void setTitleId(uint64_t title_id);

		kip::ProcessCategory getProcessCategory() const;
		void setProcessCategory(kip::ProcessCategory cat);

		const fnd::List<kip::HeaderFlags>& getFlagList() const;
		void setFlagList(const fnd::List<kip::HeaderFlags>& flags);

		byte_t getMainThreadPriority() const;
		void setMainThreadPriority(byte_t priority);

		byte_t getMainThreadCpuId() const;
		void setMainThreadCpuId(byte_t cpu_id);

		uint32_t getMainThreadStackSize() const;
		void setMainThreadStackSize(uint32_t size);

		const sCodeSegment& getTextSegmentInfo() const;
		void setTextSegmentInfo(const sCodeSegment& info);

		const sCodeSegment& getRoSegmentInfo() const;
		void setRoSegmentInfo(const sCodeSegment& info);

		const sCodeSegment& getDataSegmentInfo() const;
		void setDataSegmentInfo(const sCodeSegment& info);

		uint32_t getBssSize() const;
		void setBssSize(uint32_t size);

		const nn::hac::KernelCapabilityControl& getKernelCapabilities() const;
		void setKernelCapabilities(const KernelCapabilityControl& kc);

	private:
		const std::string kModuleName = "KERNEL_INITIAL_PROCESS_HEADER";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		std::string mName;
		uint64_t mTitleId;
		kip::ProcessCategory mProcessCategory;
		fnd::List<kip::HeaderFlags> mFlagList;
		byte_t mMainThreadPriority;
		byte_t mMainThreadCpuId;
		uint32_t mMainThreadStackSize;
		sCodeSegment mTextInfo;
		sCodeSegment mRoInfo;
		sCodeSegment mDataInfo;
		uint32_t mBssSize;
		nn::hac::KernelCapabilityControl mKernelCapabilities;
	};
}
}
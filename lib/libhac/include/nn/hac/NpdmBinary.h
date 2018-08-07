#pragma once
#include <string>
#include <fnd/List.h>
#include <fnd/ISerialisable.h>
#include <nn/hac/npdm.h>
#include <nn/hac/AccessControlInfoBinary.h>
#include <nn/hac/AccessControlInfoDescBinary.h>

namespace nn
{
namespace hac
{
	class NpdmBinary :
		public fnd::ISerialisable
	{
	public:
		NpdmBinary();
		NpdmBinary(const NpdmBinary& other);

		void operator=(const NpdmBinary& other);
		bool operator==(const NpdmBinary& other) const;
		bool operator!=(const NpdmBinary& other) const;

		// export/import binary
		void toBytes();
		void fromBytes(const byte_t* bytes, size_t len);
		const fnd::Vec<byte_t>& getBytes() const;

		// variables
		void clear();

		npdm::InstructionType getInstructionType() const;
		void setInstructionType(npdm::InstructionType type);

		npdm::ProcAddrSpaceType getProcAddressSpaceType() const;
		void setProcAddressSpaceType(npdm::ProcAddrSpaceType type);

		byte_t getMainThreadPriority() const;
		void setMainThreadPriority(byte_t priority);

		byte_t getMainThreadCpuId() const;
		void setMainThreadCpuId(byte_t cpu_id);

		uint32_t getVersion() const;
		void setVersion(uint32_t version);

		uint32_t getMainThreadStackSize() const;
		void setMainThreadStackSize(uint32_t size);

		const std::string& getName() const;
		void setName(const std::string& name);

		const std::string& getProductCode() const;
		void setProductCode(const std::string& product_code);

		const AccessControlInfoBinary& getAci() const;
		void setAci(const AccessControlInfoBinary& aci);

		const AccessControlInfoDescBinary& getAcid() const;
		void setAcid(const AccessControlInfoDescBinary& acid);
	private:
		const std::string kModuleName = "NPDM_BINARY";

		// raw binary
		fnd::Vec<byte_t> mRawBinary;

		// variables
		npdm::InstructionType mInstructionType;
		npdm::ProcAddrSpaceType mProcAddressSpaceType;
		byte_t mMainThreadPriority;
		byte_t mMainThreadCpuId;
		uint32_t mVersion;
		uint32_t mMainThreadStackSize;
		std::string mName;
		std::string mProductCode;
		AccessControlInfoBinary mAci;
		AccessControlInfoDescBinary mAcid;
	};
}
}
#include <fnd/IFile.h>
#include <fnd/MemoryBlob.h>
#include <crypto/aes.h>

class AesCtrWrappedIFile : public fnd::IFile
{
public:
	AesCtrWrappedIFile(fnd::IFile& file, const crypto::aes::sAes128Key& key, const crypto::aes::sAesIvCtr& ctr);

	size_t size();
	void seek(size_t offset);
	void read(byte_t* out, size_t len);
	void read(byte_t* out, size_t offset, size_t len);
	void write(const byte_t* out, size_t len);
	void write(const byte_t* out, size_t offset, size_t len);
private:
	const std::string kModuleName = "AesCtrWrappedIFile";
	static const size_t kAesCtrScratchSize = 0x1000000;
	static const size_t kAesCtrScratchAllocSize = kAesCtrScratchSize + crypto::aes::kAesBlockSize;

	fnd::IFile& mFile;
	crypto::aes::sAes128Key mKey;
	crypto::aes::sAesIvCtr mBaseCtr, mCurrentCtr;
	size_t mBlockOffset;

	fnd::MemoryBlob mScratch;
};
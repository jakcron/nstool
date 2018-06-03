#include <fnd/IFile.h>
#include <fnd/MemoryBlob.h>
#include <crypto/aes.h>

class AesCtrWrappedIFile : public fnd::IFile
{
public:
	AesCtrWrappedIFile(fnd::IFile* file, bool ownIfile, const crypto::aes::sAes128Key& key, const crypto::aes::sAesIvCtr& ctr);
	~AesCtrWrappedIFile();

	size_t size();
	void seek(size_t offset);
	void read(byte_t* out, size_t len);
	void read(byte_t* out, size_t offset, size_t len);
	void write(const byte_t* out, size_t len);
	void write(const byte_t* out, size_t offset, size_t len);
private:
	const std::string kModuleName = "AesCtrWrappedIFile";
	static const size_t kCacheSize = 0x10000;
	static const size_t kCacheSizeAllocSize = kCacheSize + crypto::aes::kAesBlockSize;

	bool mOwnIFile;
	fnd::IFile* mFile;
	crypto::aes::sAes128Key mKey;
	crypto::aes::sAesIvCtr mBaseCtr, mCurrentCtr;
	size_t mFileOffset;

	fnd::MemoryBlob mCache;

	void internalSeek();
};
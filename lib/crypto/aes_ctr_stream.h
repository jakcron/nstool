#pragma once
#include <string>
#include <vector>
#include <fnd/exception.h>
#include <crypto/crypto.h>

namespace crypto
{
	class AesCtrStream
	{
	public:
		AesCtrStream();
		~AesCtrStream();

		void seek(size_t offset);
		void read(size_t size, uint8_t* out);
		void read(size_t offset, size_t size, uint8_t* out);
		void write(size_t size, const uint8_t* in);
		void write(size_t offset, size_t size, const uint8_t* in);

		void AddRegion(size_t start, size_t end, const uint8_t aes_key[crypto::kAes128KeySize], const uint8_t aes_ctr[crypto::kAesBlockSize]);

	protected:
		// Virtual methods for implementation of seek/read/write
		virtual void seek_internal(size_t offset) = 0;
		virtual void read_internal(size_t size, size_t& read_len, uint8_t* out) = 0;
		virtual void write_internal(size_t size, size_t& write_len, const uint8_t* in) = 0;

	private:
		const std::string kModuleName = "AES_CTR_STREAM";
		static const size_t kIoBufferLen = 0x10000;

		// private implementation of crypto region
		class CryptRegion
		{
		public:
			// stubbed constructor
			CryptRegion() :
				start_(0),
				end_(0),
				is_plaintext_(true)
			{
				CleanUp();
			}

			// plaintext constructor
			CryptRegion(size_t start, size_t end) :
				start_(start),
				end_(end),
				is_plaintext_(true)
			{
				CleanUp();
			}

			// encrypted constructor
			CryptRegion(size_t start, size_t end, const uint8_t aes_key[crypto::kAes128KeySize], const uint8_t aes_ctr[crypto::kAesBlockSize]) :
				start_(start),
				end_(end),
				is_plaintext_(false)
			{
				CleanUp();
				memcpy(aes_key_, aes_key, crypto::kAes128KeySize);
				memcpy(ctr_init_, aes_ctr, crypto::kAesBlockSize);
				memcpy(ctr_, ctr_init_, crypto::kAesBlockSize);
			}

			// destructor
			~CryptRegion()
			{
				CleanUp();
			}

			size_t start() const { return start_; }
			size_t end() const { return end_; }
			size_t size() const { return end_ - start_; }
			size_t remaining_size(size_t start) const { return end_ - start; }
			const uint8_t* aes_key() const { return aes_key_; }
			uint8_t* aes_ctr() { return ctr_; }

			bool is_in_region(size_t start) const { return start >= start_ && start < end_; }
			bool is_in_region(size_t start, size_t end) const { return is_in_region(start) && end > start_ && end <= end_; }

			void UpdateAesCtr(size_t start)
			{
				if (is_in_region(start))
					crypto::AesIncrementCounter(ctr_init_, ((start - start_) >> 4), ctr_);
			}

			void GenerateXorpad(size_t start, size_t size, uint8_t* out)
			{
				// don't operate if requested size exceeds region size
				if (is_in_region(start, start + size) == false)
				{
					return;
				}

				if (is_plaintext_ == true)
				{
					memset(out, 0, size);
					return;
				}

				// parameters
				size_t block_offset = (start - start_) & 0xf;
				size_t block_num = size >> 4;
				for (size_t pos = 0; pos < block_num; pos += (kPadBufferLen >> 4))
				{
					// clear pad buffer
					memset(pad_buffer_, 0, kPadBufferCapacity);

					// encrypt pad buffer to create xorpad
					UpdateAesCtr(start + (pos << 4));
					crypto::AesCtr(pad_buffer_, kPadBufferCapacity, aes_key(), aes_ctr(), pad_buffer_);

					// determine the number of blocks to copy to xorpad
					size_t copy_size = kPadBufferLen < ((block_num - pos) << 4) ? kPadBufferLen : ((block_num - pos) << 4);

					// copy 
					memcpy(out + (pos << 4), pad_buffer_ + block_offset, copy_size);
				}
			}
		private:
			static const size_t kPadBufferLen = 0x10000;
			static const size_t kPadBufferCapacity = kPadBufferLen + crypto::kAesBlockSize; // has an extra block to accomodate non block aligned starts

			size_t start_;
			size_t end_;
			bool is_plaintext_;
			uint8_t aes_key_[crypto::kAes128KeySize];
			uint8_t ctr_init_[crypto::kAesBlockSize];
			uint8_t ctr_[crypto::kAesBlockSize];
			uint8_t pad_buffer_[kPadBufferCapacity];

			void CleanUp()
			{
				memset(aes_key_, 0, crypto::kAes128KeySize);
				memset(ctr_init_, 0, crypto::kAesBlockSize);
				memset(ctr_, 0, crypto::kAesBlockSize);
			}
		};



		inline void xor_data(size_t size, const uint8_t* data1, const uint8_t* data2, uint8_t* out)
		{
			for (size_t idx = 0; idx < size; idx++)
			{
				out[idx] = data1[idx] ^ data2[idx];
			}
		}

		// Crypto Regions
		size_t offset_;
		std::vector<CryptRegion> regions_;

		// IO Buffer
		uint8_t io_buffer_[kIoBufferLen];
		uint8_t pad_buffer_[kIoBufferLen];

		void GenerateXorPad(size_t start);
	};
}
#include "aes_ctr_stream.h"

using namespace crypto::aes;

AesCtrStream::AesCtrStream()
{
}


AesCtrStream::~AesCtrStream()
{
}

void AesCtrStream::seek(size_t offset)
{
	offset_ = offset;
	seek_internal(offset_);
}

void AesCtrStream::read(size_t size, uint8_t * out)
{
	size_t read_len = 0;
	size_t read_size = 0;
	for (size_t pos = 0; pos < size; pos += read_size, offset_ += read_size)
	{
		// calculate read size
		read_size = (size - pos) < kIoBufferLen ? (size - pos) : kIoBufferLen;		
		
		// read data
		read_internal(read_size, read_len, io_buffer_);
		if (read_size != read_len)
		{
			throw fnd::Exception(kModuleName, "Stream read length unexpected");
		}

		// crypt data
		GenerateXorPad(offset_);
		xor_data(read_size, pad_buffer_, io_buffer_, out + pos);
	}
}

void AesCtrStream::read(size_t offset, size_t size, uint8_t * out)
{
	seek(offset);
	read(size, out);
}

void AesCtrStream::write(size_t size, const uint8_t * in)
{
	size_t write_len = 0;
	size_t write_size = 0;
	for (size_t pos = 0; pos < size; pos += write_size, offset_ += write_size)
	{
		// calculate write size
		write_size = (size - pos) < kIoBufferLen ? (size - pos) : kIoBufferLen;

		// crypt data
		GenerateXorPad(offset_);
		xor_data(write_size, pad_buffer_, in + pos, io_buffer_);
		
		// write data
		write_internal(write_size, write_len, io_buffer_);
		if (write_size != write_len)
		{
			throw fnd::Exception(kModuleName, "Stream write length unexpected");
		}
	}
}

void AesCtrStream::write(size_t offset, size_t size, const uint8_t * in)
{
	seek(offset);
	write(size, in);
}

void AesCtrStream::AddRegion(size_t start, size_t end, const uint8_t aes_key[kAes128KeySize], const uint8_t aes_ctr[kAesBlockSize])
{
	if (start >= end)
	{
		throw fnd::Exception(kModuleName, "Illegal start/end position");
	}
	if (aes_key == nullptr || aes_ctr == nullptr)
	{
		throw fnd::Exception(kModuleName, "Illegal aes configuration (nullptr)");
	}

	regions_.push_back(CryptRegion(start, end, aes_key, aes_ctr));
}

void AesCtrStream::GenerateXorPad(size_t start)
{
	size_t pad_size = 0;
	for (size_t pos = 0; pos < kIoBufferLen; pos += pad_size)
	{
		CryptRegion* cur_region = nullptr;
		CryptRegion* next_region = nullptr;
		for (size_t idx = 0; idx < regions_.size(); idx++)
		{
			if (regions_[idx].is_in_region(start + pos))
			{
				cur_region = &regions_[idx];
			}
			else if (regions_[idx].start() > (start + pos) && (next_region == nullptr || next_region->start() > regions_[idx].start()))
			{
				next_region = &regions_[idx];
			}
		}
		
		// if this exists in the a crypto region
		if (cur_region != nullptr)
		{
			pad_size = cur_region->remaining_size(start + pos);
			if (pad_size > kIoBufferLen - pos)
			{
				pad_size = kIoBufferLen - pos;
			}
			cur_region->GenerateXorpad(start + pos, pad_size, pad_buffer_ + pos);
		}

		// there is a crypto region ahead, bridge the gap
		else if (next_region != nullptr)
		{
			pad_size = next_region->start() - (start + pos);
			if (pad_size > kIoBufferLen - pos)
			{
				pad_size = kIoBufferLen - pos;
			}
			memset(pad_buffer_ + pos, 0, pad_size);
		}
		// there are no more crypto regions
		else
		{
			pad_size = kIoBufferLen - pos;
			memset(pad_buffer_ + pos, 0, pad_size);
		}
	}

}

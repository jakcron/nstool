#pragma once
#include <string>
#include <fnd/MemoryBlob.h>
#include <fnd/ISerialiseableBinary.h>
#include <es/ticket.h>

namespace es
{
	class TicketBinary
		: public fnd::ISerialiseableBinary
	{
	public:
		TicketBinary();
		TicketBinary(const TicketBinary& other);

		void operator=(const TicketBinary& other);
		bool operator==(const TicketBinary& other) const;
		bool operator!=(const TicketBinary& other) const;

		void importBinary(byte_t* src, size_t size);
		void exportBinary();

		const byte_t* getBytes() const;
		size_t getSize() const;

		
	private:
	};
}
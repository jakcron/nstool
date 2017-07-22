#include <cstdio>
#include <crypto/aes.h>
#include <fnd/io.h>
#include <fnd/memory_blob.h>
#include <es/ETicketBody_V2.h>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: tiktool <file>\n");
		return 1;
	}

	try
	{
		fnd::MemoryBlob file;
		fnd::io::readFile(argv[1], file);

		// import
		es::ETicketBody_V2 body;
		body.importBinary(file.getBytes() + 0x140, file.getSize() - 0x140);

		printf("Issuer: %s\n", body.getIssuer().c_str());
		

	} catch (const fnd::Exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}
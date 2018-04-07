#include <fnd/SimpleTextOutput.h>
#include <cstdio>

void fnd::SimpleTextOutput::hxdStyleDump(const byte_t* data, size_t len, size_t row_len, size_t byte_grouping_size)
{
	// iterate over blocks
	for (size_t i = 0; i < (len / row_len); i++)
	{
		// for block i print each byte
		for (size_t j = 0; j < row_len; j++)
		{
			printf("%02X", data[(i * row_len) + j]);
			if (((j+1) % byte_grouping_size) == 0) 
			{
				putchar(' ');
			}
		}
		printf(" ");
		for (size_t j = 0; j < row_len; j++)
		{
			printf("%c", isalnum(data[(i * row_len) + j]) ? data[(i * row_len) + j] : '.');
		}
		printf("\n");
	}
}

void fnd::SimpleTextOutput::hxdStyleDump(const byte_t* data, size_t len)
{
	hxdStyleDump(data, len, kDefaultRowLen, kDefaultByteGroupingSize);
}

void fnd::SimpleTextOutput::hexDump(const byte_t* data, size_t len, size_t row_len, size_t indent_len)
{
	for (size_t i = 0; i < len; i++)
	{
		if ((i % row_len) == 0)
		{
			if (i > 0)
				putchar('\n');
			for (size_t j = 0; j < indent_len; j++)
			{
				putchar(' ');
			}
		}
		printf("%02X", data[i]);
		if ((i+1) >= len)
		{
			putchar('\n');
		}
		
	}
}

void fnd::SimpleTextOutput::hexDump(const byte_t* data, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		printf("%02X", data[i]);
	}
	putchar('\n');
}
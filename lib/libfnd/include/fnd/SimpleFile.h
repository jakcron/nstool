#pragma once
#include <fnd/IFile.h>
#include <string>
#include <cstdio>

namespace fnd
{
	class SimpleFile : public IFile
	{
	public:
		enum OpenMode
		{
			Read,
			Edit,
			Create
		};

		SimpleFile();
		~SimpleFile();

		void open(const std::string& path, OpenMode mode);
		bool isOpen() const;
		void close();
		size_t size();
		void seek(size_t offset);
		size_t pos();
		void read(byte_t* out, size_t offset, size_t len);
		void write(const byte_t* out, size_t offset, size_t len);
	
	private:
		const std::string kModuleName = "SimpleFile";

		bool mOpen;
		OpenMode mMode;
		FILE* mFp;

		const char* getOpenModeStr(OpenMode mMode);
	};
}


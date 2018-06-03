#pragma once
#include <fnd/IFile.h>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdio>
#endif

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
		SimpleFile(const std::string& path, OpenMode mode);
		~SimpleFile();

		void open(const std::string& path, OpenMode mode);
		bool isOpen() const;
		void close();
		size_t size();
		void seek(size_t offset);
		size_t pos();
		void read(byte_t* out, size_t len);
		void read(byte_t* out, size_t offset, size_t len);
		void write(const byte_t* out, size_t len);
		void write(const byte_t* out, size_t offset, size_t len);
	
	private:
		const std::string kModuleName = "SimpleFile";

		bool mOpen;
		OpenMode mMode;

#ifdef _WIN32
		HANDLE mFileHandle;
		DWORD getOpenModeFlag(OpenMode mode) const;
		DWORD getShareModeFlag(OpenMode mode) const;
		DWORD getCreationModeFlag(OpenMode mode) const;
#else
		FILE* mFp;
		const char* getOpenModeStr(OpenMode mode);
#endif
	};
}


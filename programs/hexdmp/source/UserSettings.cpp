#include "UserSettings.h"
#include "version.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fnd/io.h>
#include <fnd/SimpleFile.h>
#include <fnd/SimpleTextOutput.h>
#include <fnd/Vec.h>


UserSettings::UserSettings()
{}

void UserSettings::parseCmdArgs(const std::vector<std::string>& arg_list)
{
	sCmdArgs args;
	populateCmdArgs(arg_list, args);
	populateUserSettings(args);
}

void UserSettings::showHelp()
{
	printf("%s v%d.%d.%d (C) %s\n", APP_NAME, VER_MAJOR, VER_MINOR, VER_PATCH, AUTHORS);
	printf("Built: %s %s\n\n", __TIME__, __DATE__);
	
	printf("Usage: %s [options... ] <file>\n", BIN_NAME);
	printf("\n  General Options:\n");
	//printf("      -t, --type        Specify input file type. [raw]\n");
	printf("      -a, --showascii   Show data as ASCII.\n");
	printf("      -p, --showoffset  Show data offset.\n");
	printf("      -v, --verbose     Verbose output.\n");
	printf("      -g, --grouping    Specify byte grouping size (default 1).\n");
	printf("      -o, --read-offset Specify read offset (default 0).\n");
	printf("      -s, --read-size   Specify read size (default 0x200).\n");
}

const std::string UserSettings::getInputPath() const
{
	return mInputPath;
}

FileType UserSettings::getFileType() const
{
	return mFileType;
}

CliOutputMode UserSettings::getCliOutputMode() const
{
	return mOutputMode;
}

bool UserSettings::isShowAsciiRepresentation() const
{
	return mShowAsciiRepresentation;
}

bool UserSettings::isShowOffset() const
{
	return mShowOffset;
}

const sOptional<size_t>& UserSettings::getByteGroupingSize() const
{
	return mByteGroupingSize;
}

const sOptional<size_t>& UserSettings::getReadOffset() const
{
	return mReadOffset;
}

const sOptional<size_t>& UserSettings::getReadSize() const
{
	return mReadSize;
}

void UserSettings::populateCmdArgs(const std::vector<std::string>& arg_list, sCmdArgs& cmd_args)
{
	// show help text
	if (arg_list.size() < 2)
	{
		showHelp();
		throw fnd::Exception(kModuleName, "Not enough arguments.");
	}

	cmd_args.input_path = arg_list.back();

	for (size_t i = 1; i < arg_list.size(); i++)
	{
		if (arg_list[i] == "-h" || arg_list[i] == "--help")
		{
			showHelp();
			throw fnd::Exception(kModuleName, "Nothing to do.");
		}
	}

	for (size_t i = 1; i+1 < arg_list.size(); i++)
	{
		bool hasParamter = arg_list[i+1][0] != '-' && i+2 < arg_list.size();

		if (arg_list[i] == "-a" || arg_list[i] == "--showascii")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.show_ascii = true;
		}

		else if (arg_list[i] == "-p" || arg_list[i] == "--showoffset")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.show_offset = true;
		}

		else if (arg_list[i] == "-v" || arg_list[i] == "--verbose")
		{
			if (hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " does not take a parameter.");
			cmd_args.verbose = true;
		}

		else if (arg_list[i] == "-t" || arg_list[i] == "--type")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.file_type = arg_list[i+1];
		}

		else if (arg_list[i] == "-g" || arg_list[i] == "--grouping")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.byte_grouping = arg_list[i+1];
		}

		else if (arg_list[i] == "-o" || arg_list[i] == "--read-offset")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.read_offset = arg_list[i+1];
		}

		else if (arg_list[i] == "-s" || arg_list[i] == "--read-size")
		{
			if (!hasParamter) throw fnd::Exception(kModuleName, arg_list[i] + " requries a parameter.");
			cmd_args.read_size = arg_list[i+1];
		}
		else
		{
			throw fnd::Exception(kModuleName, arg_list[i] + " is not recognised.");
		}

		i += hasParamter;
	}
}

void UserSettings::populateUserSettings(sCmdArgs& args)
{
	// check invalid input
	if (args.input_path.isSet == false)
		throw fnd::Exception(kModuleName, "No input file specified");
	
	// save arguments
	mInputPath = *args.input_path;
	mShowAsciiRepresentation = args.show_ascii.isSet;
	mShowOffset = args.show_offset.isSet;

	if (args.byte_grouping.isSet)
		mByteGroupingSize = strtoul((*args.byte_grouping).c_str(), nullptr, 0);
	if (args.read_offset.isSet)
		mReadOffset = strtoul((*args.read_offset).c_str(), nullptr, 0);
	if (args.read_size.isSet)
		mReadSize = strtoul((*args.read_size).c_str(), nullptr, 0);

	// determine output mode
	mOutputMode = _BIT(OUTPUT_BASIC);
	if (args.verbose.isSet)
	{
		mOutputMode |= _BIT(OUTPUT_KEY_DATA);
		mOutputMode |= _BIT(OUTPUT_LAYOUT);
		mOutputMode |= _BIT(OUTPUT_EXTENDED);
	}

	// determine input file type
	if (args.file_type.isSet)
		mFileType = getFileTypeFromString(*args.file_type);
	else
		mFileType = FILE_RAW;
	
	// check is the input file could be identified
	if (mFileType == FILE_INVALID)
		throw fnd::Exception(kModuleName, "Unknown file type.");
}

FileType UserSettings::getFileTypeFromString(const std::string& type_str)
{
	std::string str = type_str;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	
	FileType type;
	if (str == "raw")
		type = FILE_RAW;
	else
		type = FILE_INVALID;

	return type;
}
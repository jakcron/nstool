#pragma once
#include <tc/types.h>
#include <tc/Exception.h>
#include <tc/Optional.h>
#include <tc/io.h>
#include <tc/io/IOUtil.h>
#include <tc/cli.h>
#include <fmt/core.h>


namespace nstool {

struct CliOutputMode
{
	bool show_basic_info;
	bool show_extended_info;
	bool show_layout;
	bool show_keydata;

	CliOutputMode() : show_basic_info(false), show_extended_info(false), show_layout(false), show_keydata(false)
	{}

	CliOutputMode(bool show_basic_info, bool show_extended_info, bool show_layout, bool show_keydata) : show_basic_info(show_basic_info), show_extended_info(show_extended_info), show_layout(show_layout), show_keydata(show_keydata)
	{}
};

struct ArchiveJob
{
	/* Common Job Properties */
	enum class JobAction {
		DoNothing,
		ListFileTree,
		Extract
	};
	JobAction job_action; // action taken by job processor
	tc::io::Path archive_path; 	// path within archive to process

	/* Extract Job Properties */
	tc::io::Path extract_base_path; // path to base directory to extract
	bool preserve_archive_path; // recreate archive path when extracting 

	ArchiveJob() : job_action(JobAction::DoNothing), archive_path(), extract_base_path(), preserve_archive_path(false)
	{}

	ArchiveJob(JobAction job_action, const tc::io::Path& archive_path, const tc::io::Path& extract_base_path, bool preserve_archive_path) :
		job_action(job_action),
		archive_path(archive_path),
		extract_base_path(extract_base_path),
		preserve_archive_path(preserve_archive_path)
	{}
};

struct ExtractJob {
	tc::io::Path virtual_path;
	tc::io::Path extract_path;
};

}
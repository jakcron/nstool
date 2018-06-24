#include <fnd/io.h>
#include <fnd/StringConv.h>
#include <fnd/SimpleFile.h>
#include <fstream>
#ifdef _WIN32
#include <direct.h>
#include <cstdlib>
#else
#include <sys/stat.h>
#endif

using namespace fnd;

size_t io::getFileSize(const std::string& path)
{
	std::ifstream f;
	f.open(path, std::ios_base::binary | std::ios_base::in);
	if (!f.good() || f.eof() || !f.is_open()) { return 0; }
	f.seekg(0, std::ios_base::beg);
	std::ifstream::pos_type begin_pos = f.tellg();
	f.seekg(0, std::ios_base::end);
	return static_cast<size_t>(f.tellg() - begin_pos);
}

void io::makeDirectory(const std::string& path)
{
#ifdef _WIN32
	std::u16string wpath = fnd::StringConv::ConvertChar8ToChar16(path);
	_wmkdir((wchar_t*)wpath.c_str());
#else
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

void fnd::io::getEnvironVar(std::string & var, const std::string & key)
{
#ifdef _WIN32
	char* var_tmp = nullptr;
	size_t var_len = 0;
	_dupenv_s(&var_tmp, &var_len, key.c_str());

	if (var_len > 0)
	{
		var = std::string(var_tmp);
		free(var_tmp);
	}
#else
	char* var_tmp = nullptr;

	var_tmp = getenv(key.c_str());

	if (var_tmp != nullptr)
	{
		var = std::string(var_tmp);
	}
#endif
}

void fnd::io::appendToPath(std::string& base, const std::string& add)
{
	if (add.empty())
		return;

	if (base.empty())
	{
		base = add;
	}
	else
	{
		if (base[base.length()-1] != io::kPathDivider[0])
			base += io::kPathDivider;
		base += add;
	}
}

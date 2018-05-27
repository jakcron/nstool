#include <fnd/Exception.h>

using namespace fnd;

Exception::Exception() noexcept :
	what_(""),
	module_(""),
	error_("")
{

}

Exception::Exception(const std::string & what) noexcept :
	what_(what),
	module_(""),
	error_(what)
{
}

Exception::Exception(const std::string & module, const std::string & what) noexcept :
	what_(""),
	module_(module),
	error_(what)
{
	if (module_.length() > 0)
	{
		what_ = "[" + module_ + " ERROR] " + error_;
	}
	else
	{
		what_ = error_;
	}
}

const char* Exception::what() const noexcept 
{
	return what_.c_str();
}

const char* Exception::module() const noexcept
{
	return module_.c_str();
}

const char * fnd::Exception::error() const noexcept
{
	return error_.c_str();
}

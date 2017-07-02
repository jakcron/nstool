#include "exception.h"

using namespace fnd;

Exception::Exception() noexcept :
	what_(""),
	module_(""),
	level_(E_FATAL)
{

}

Exception::Exception(const std::string & what) noexcept :
	what_(what),
	module_(""),
	level_(E_FATAL)
{
}

Exception::Exception(const std::string & what, ExceptionLevel level) noexcept :
	what_(what),
	module_(""),
	level_(level)
{
}

Exception::Exception(const std::string & module, const std::string & what) noexcept :
	what_(what),
	module_(module),
	level_(E_FATAL)
{
}

Exception::Exception(const std::string & module, const std::string & what, ExceptionLevel level) noexcept :
what_(what),
	module_(module),
	level_(level)
{
}

Exception::~Exception()
{
}

const char* Exception::what() const noexcept 
{
	return what_.c_str();
}

const char* Exception::module() const noexcept
{
	return module_.c_str();
}

bool Exception::is_fatal() const noexcept
{
	return level_ == E_FATAL;
}

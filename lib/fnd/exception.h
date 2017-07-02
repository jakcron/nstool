#pragma once
#include <exception>
#include <string>

namespace fnd
{
	class Exception : public std::exception
	{
	public:
		enum ExceptionLevel
		{
			E_RECOVERABLE,
			E_FATAL,
		};

		Exception() noexcept;
		Exception(const std::string& what) noexcept;
		Exception(const std::string& what, ExceptionLevel level) noexcept;
		Exception(const std::string& module, const std::string& what) noexcept;
		Exception(const std::string& module, const std::string& what, ExceptionLevel level) noexcept;


		~Exception();

		const char* what() const noexcept;
		const char* module() const noexcept;
		bool is_fatal() const noexcept;
	private:
		std::string what_;
		std::string module_;
		ExceptionLevel level_;
	};
}



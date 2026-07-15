// log.hpp

#pragma once

#include <iostream>
#include <sstream>

namespace triton
{
	template<typename T>
	inline void Print(const T& message)
	{
		std::cout << message << std::endl;
	}

	template<typename T>
	inline void Print(const T& message, std::ostream& stream)
	{
		stream << message << std::endl;
	}

	inline void Print(std::ostringstream& stream)
	{
		std::cout << stream.str();
	}
}
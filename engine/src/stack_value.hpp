#pragma once

#include "types.hpp"

namespace triton
{
	template <typename TValue>
	class cStack;
	template <typename TKey, typename TValue>
	class cHashTable;

	/*class cStackValue
	{
		template <typename>
		friend class cStack;
		template <typename, typename>
		friend class cHashTable;

		types::u32 chunk = 0;
		types::u32 localPosition = 0;
		types::u32 globalPosition = 0;
	};*/
}
#pragma once

#include "types.hpp"

namespace triton
{
	template <typename TValue>
	class XDynamicArray;
	template <typename TKey, typename TValue>
	class cHashTable;

	/*class XDynamicArrayValue
	{
		template <typename>
		friend class XDynamicArray;
		template <typename, typename>
		friend class cHashTable;

		types::u32 chunk = 0;
		types::u32 localPosition = 0;
		types::u32 globalPosition = 0;
	};*/
}
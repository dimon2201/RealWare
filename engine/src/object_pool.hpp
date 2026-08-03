// object_pool.hpp

#pragma once

#include "object_allocator.hpp"
#include "static_vertex_pool.hpp"
#include "types.hpp"

namespace triton
{
	class CPool final
	{
		inline static XStaticVertexPool* _staticVertexPool = nullptr;

	public:
		static void Initialize(cContext* context)
		{
			_staticVertexPool = CObjectAllocator::Create<XStaticVertexPool>(
				64,
				context,
				types::K_TRUE,
				0,
				cBuffer::eType::VERTEX
			);
		}

		static void Free()
		{
		}
	};
}
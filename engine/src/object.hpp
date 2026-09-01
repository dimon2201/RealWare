// object.hpp

#pragma once

#include <cstring>
#include <array>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "tag.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class XDataBuffer;
	class cGameObject;

	using ClassType = ::std::string;

	#define TRITON_OBJECT(typeName) \
		public: \
			static ClassType GetTypeStatic() { return #typeName; } \
			virtual ClassType GetType() const override { return GetTypeStatic(); } \

	#define TRITON_CLASS_NAME(className) \
		public: \
			static std::string GetTypeStatic() { return #className; } \
			virtual ClassType GetType() const { return GetTypeStatic(); } \
		private: \

	class cIdentifier
	{
	public:
		static cTag Generate(const ::std::string& seed);
	};

	class iObject;
	class cObjectPtr
	{
	public:
		iObject* object = nullptr;
	};

	class iObject
	{
		template <typename T>
		friend class cIdVector;
		template <typename T>
		friend class cFactory;
		friend class cMemoryAllocator;

	protected:
		cContext* _context = nullptr;
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		types::boolean _allocatedUsingMemAllocator = types::K_FALSE;
		types::s32 _poolIndex = -1;
		cTag _id;

	public:

		iObject() = default; // TODO: remove this <<<<<-----

		explicit iObject(cContext* context, types::s32 poolIndex) : _context(context), _poolIndex(poolIndex) {}
		virtual ~iObject() = default;

		// TODO: Copying of object is now removed, bring it back
		//iObject(const iObject& rhs) = delete;
		//iObject& operator=(const iObject& rhs) = delete;

		virtual ClassType GetType() const = 0;

		void Subscribe(eEventType type, EventFunction&& function);
		void Unsubscribe(eEventType type);
		void Send(eEventType type);
		void Send(eEventType type, XDataBuffer* data);

		inline cContext* GetContext() const { return _context; }
		inline const cTag& GetID() const { return _id; }

		inline types::s32 GetPoolIndex() const { return _poolIndex; }
	};
}
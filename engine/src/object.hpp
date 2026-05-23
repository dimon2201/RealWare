// object.hpp

#pragma once

#include <string>
#include <array>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "tag.hpp"
#include "stack_value.hpp"
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

	class iObject : public cStackValue
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
		cTag _id;

	public:
		explicit iObject(cContext* context) : _context(context) {}
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
	};
}
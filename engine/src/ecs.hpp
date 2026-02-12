#pragma once

#include <type_traits>
#include "object.hpp"
#include "engine.hpp"
#include "application.hpp"
#include "hash_table.hpp"
#include "stack.hpp"
#include "entity.hpp"
#include "scene.hpp"
#include "single_value.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
}

namespace triton::ecs
{
	// TODO: move cSceneStorage to separate "scene_storage.hpp" file
	class cSceneStorage : public iObject
	{
		TRITON_OBJECT(cSceneStorage)

		cStack<cScene>* _scenes = nullptr;
		cHashTable<std::string, cSingleValue>* _sceneIndices = nullptr;

	public:
		struct sFindResult
		{
			types::boolean isOK = types::K_FALSE;
			types::usize index = 0;
		};

		explicit cSceneStorage(triton::cContext* context);
		virtual ~cSceneStorage() override final;

		cScene* Create(const std::string& name);
		cScene* Get(const std::string& name);
		cScene* Get(types::usize index);
		void Destroy(const std::string& name);
		sFindResult FindEntity(entity ent);
	};

	class cECSSystem : public iObject
	{
		TRITON_OBJECT(cECSSystem)

		cSceneStorage* _scenes = nullptr;

	public:
		explicit cECSSystem(triton::cContext* context) : iObject(context)
		{
			_scenes = _context->Create<cSceneStorage>(_context);
		}

		virtual ~cECSSystem() override final
		{
			_context->Destroy<cSceneStorage>(_scenes);
		}

		inline cSceneStorage* GetScenes() const { return _scenes; }
	};
}
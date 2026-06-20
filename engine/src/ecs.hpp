#pragma once

// TODO: remove "engine.hpp" and "application.hpp" from "ecs.hpp",
// ECS module must be independent from engine and application modules

#include <type_traits>
#include "object.hpp"
#include "scene.hpp"
#include "single_value.hpp"

namespace triton
{
	class cContext;
	template <typename T>
	class cStack;
	template <typename TKey, typename TValue>
	class cHashTable;

	namespace ecs
	{
		// TODO: move cSceneStorage to separate "scene_storage.hpp" file
		class cSceneStorage : public iObject
		{
			TRITON_OBJECT(cSceneStorage)

			cStack<cScene>* _scenes = nullptr;
			cHashTable<::std::string, cSingleValue>* _sceneIndices = nullptr;

		public:
			struct sFindResult
			{
				types::boolean isOK = types::K_FALSE;
				types::usize index = 0;
			};

			explicit cSceneStorage(triton::cContext* context);
			virtual ~cSceneStorage() override final;
				
			const cScene& Create(const ::std::string& name);
			const cScene& Get(const ::std::string& name);
			const cScene& Get(types::usize index);
			void Destroy(const ::std::string& name);
			sFindResult FindEntity(entity ent);
		};

		class cECSSystem : public iObject
		{
			TRITON_OBJECT(cECSSystem)

			cSceneStorage* _scenes = nullptr;

		public:
			explicit cECSSystem(cContext* context) : iObject(context)
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
}
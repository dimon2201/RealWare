// scene.hpp

#pragma once

#include "object.hpp"
#include "entity.hpp"
#include "hash_table.hpp"
#include "component_storage.hpp"
#include "single_value.hpp"

namespace triton::ecs
{
	class cScene : public iObject
	{
		// TODO: Create component storages for each component type
		// cComponentStorage<sTransform> _transforms;
		// ...
		// cComponentStorage<sHealth> _healths;

		TRITON_OBJECT(cScene)

		::std::string _name = "";
		cHashTable<entity, cSingleValue>* _isEntityExist = nullptr; // TODO: make this member field not of a pointer type
		cComponentStorage<components::STransformComponent>* _transforms = nullptr;

	public:
		explicit cScene(cContext* context, const ::std::string& name);
		virtual ~cScene() override final;

		entity CreateEntity();
		void DestroyEntity(entity ent);
		types::boolean IsEntityExist(entity ent);
		components::STransformComponent* CreateTransformComponent(entity ent);
		void DestroyTransformComponent(entity ent);

		inline const ::std::string& GetName() const
		{
			return _name;
		}
	};
}
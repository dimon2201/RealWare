// scene.hpp

#pragma once

#include "object.hpp"
#include "entity.hpp"
#include "hash_table.hpp"
#include "component_storage.hpp"
#include "single_value.hpp"
#include "render_data.hpp"

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
		cComponentStorage<components::STransformComponent>* _transformComponents = nullptr;
		cComponentStorage<components::SRenderInstanceComponent>* _renderInstanceComponents = nullptr;
		cComponentStorage<components::SGeometryComponent>* _geometryComponents = nullptr;
		XDynamicArray<components::SRenderInstanceComponent>* _renderInstancesStatic = nullptr;
		XDynamicArray<components::SRenderInstanceComponent>* _renderInstancesDynamic = nullptr;

	public:
		explicit cScene(cContext* context, const ::std::string& name);
		virtual ~cScene() override final;

		entity CreateEntity();
		void DestroyEntity(entity ent);
		types::boolean IsEntityExist(entity ent);
		components::STransformComponent* CreateTransformComponent(entity ent);
		components::SRenderInstanceComponent* CreateRenderInstanceComponent(entity ent);
		components::SGeometryComponent* CreateGeometryComponent(entity ent);
		void DestroyTransformComponent(entity ent);
		void DestroyRenderInstanceComponent(entity ent);
		SRenderData BuildRenderDataStatic();
		SRenderData BuildRenderDataDynamic();

		inline const ::std::string& GetName() const
		{
			return _name;
		}
	};
}
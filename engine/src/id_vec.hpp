#pragma once

#include <vector>
#include "types.hpp"

namespace realware
{
	namespace app
	{
		class cApplication;
	}

	namespace utils
	{
		struct sIdVecObject
		{
			std::string ID = "";
			app::cApplication* App = nullptr;
			types::boolean IsDeleted = types::K_FALSE;
		};

		template <typename T>
		class cIdVec
		{
		public:
			explicit cIdVec(const app::cApplication* const app, const types::usize maxObjectCount);
			~cIdVec() = default;

			template<typename... Args>
			T* Add(const std::string& id, Args&&... args);
			T* Find(const std::string& id);
			void Delete(const std::string& id);

			inline std::vector<T>& GetObjects() { return _objects; }

		private:
			app::cApplication* _app = nullptr;
			types::usize _maxObjectCount = 0;
			std::vector<T> _objects = {};
		};

		template<typename T>
		cIdVec<T>::cIdVec(const app::cApplication* const app, const types::usize maxObjectCount) : _app((app::cApplication*)app), _maxObjectCount(maxObjectCount)
		{
			_objects.reserve(maxObjectCount);
		}

		template<typename T>
		template<typename... Args>
		T* cIdVec<T>::Add(const std::string& id, Args&&... args)
		{
			const types::usize objectCount = _objects.size();

			for (types::usize i = 0; i < objectCount; i++)
			{
				if (_objects[i].IsDeleted == K_TRUE)
				{
					_objects[i] = T(std::forward<Args>(args)...);
					_objects[i].ID = id;
					_objects[i].App = _app;
					_objects[i].IsDeleted = K_FALSE;

					return &_objects[i];
				}
			}

			if (objectCount < _maxObjectCount)
			{
				_objects.emplace_back(std::forward<Args>(args)...);

				T& object = _objects.back();
				object.ID = id;
				object.App = _app;
				object.IsDeleted = K_FALSE;

				return &object;
			}

			return nullptr;
		}

		template<typename T>
		T* cIdVec<T>::Find(const std::string& id)
		{
			const types::usize objectCount = _objects.size();

			for (types::usize i = 0; i < objectCount; i++)
			{
				if (_objects[i].IsDeleted == K_FALSE && _objects[i].ID == id)
					return &_objects[i];
			}

			return nullptr;
		}

		template<typename T>
		void cIdVec<T>::Delete(const std::string& id)
		{
			T* object = Find(id);
			if (object != nullptr)
				object->IsDeleted = K_TRUE;
		}
	}
}
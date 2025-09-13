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
		class cIdVecObject
		{
		public:
			cIdVecObject() = default;
			~cIdVecObject() = default;

			const std::string& GetID() const { return _id; }
			app::cApplication* GetApp() const { return _app; }
			types::boolean IsDeleted() const { return _isDeleted; }

			inline void SetID(const std::string& id) { _id = id; }
			inline void SetApp(const app::cApplication* const app) { _app = (app::cApplication*)app; }
			inline void SetIsDeleted(const types::boolean isDeleted) { _isDeleted = isDeleted; }

		private:
			std::string _id = "";
			app::cApplication* _app = nullptr;
			types::boolean _isDeleted = types::K_FALSE;
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
				if (_objects[i].IsDeleted() == K_TRUE)
				{
					_objects[i] = T(std::forward<Args>(args)...);
					_objects[i].SetID(id);
					_objects[i].SetApp(_app);
					_objects[i].SetIsDeleted(K_FALSE);

					return &_objects[i];
				}
			}

			if (objectCount < _maxObjectCount)
			{
				_objects.emplace_back(std::forward<Args>(args)...);

				T& object = _objects.back();
				object.SetID(id);
				object.SetApp(_app);
				object.SetIsDeleted(K_FALSE);

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
				if (_objects[i].IsDeleted() == K_FALSE && _objects[i].GetID() == id)
					return &_objects[i];
			}

			return nullptr;
		}

		template<typename T>
		void cIdVec<T>::Delete(const std::string& id)
		{
			T* object = Find(id);
			if (object != nullptr)
				object->SetIsDeleted(K_TRUE);
		}
	}
}
// math.hpp

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
	class cQuaternion;
	class cMatrix4;
	class cTransform;

	class cVector2
	{
		friend class cVector2;

		glm::vec2 _vec = glm::vec2(0.0f);

	public:
		explicit cVector2(const glm::vec2& vec);
		explicit cVector2(types::f32 value);
		explicit cVector2(types::f32 x, types::f32 y);
		~cVector2() = default;

		cVector2 operator+(const cVector2& vec) const;
		cVector2 operator-(const cVector2& vec) const;
		cVector2 operator*(const cVector2& vec) const;
		cVector2 operator/(const cVector2& vec) const;
		cVector2 operator+(types::f32 val) const;
		cVector2 operator-(types::f32 val) const;
		cVector2 operator*(types::f32 val) const;
		cVector2 operator/(types::f32 val) const;

		inline types::f32 GetX() const { return _vec.x; }
		inline types::f32 GetY() const { return _vec.y; }
		inline void SetX(types::f32 value) { _vec.x = value; }
		inline void SetY(types::f32 value) { _vec.y = value; }
		inline void AddX(types::f32 value) { _vec.x += value; }
		inline void AddY(types::f32 value) { _vec.y += value; }
	};

	class cVector3
	{
		friend class cVector3;
		friend class cQuaternion;
		friend class cMatrix4;
		friend class cTransform;

	public: // TODO: remove this
		glm::vec3 _vec = glm::vec3(0.0f);

	public:
		explicit cVector3(const glm::vec3& vec);
		explicit cVector3(types::f32 value);
		explicit cVector3(types::f32 x, types::f32 y, types::f32 z);
		~cVector3() = default;

		cVector3 operator+(const cVector3& vec) const;
		cVector3 operator-(const cVector3& vec) const;
		cVector3 operator*(const cVector3& vec) const;
		cVector3 operator/(const cVector3& vec) const;
		cVector3 operator+(types::f32 val) const;
		cVector3 operator-(types::f32 val) const;
		cVector3 operator*(types::f32 val) const;
		cVector3 operator/(types::f32 val) const;

		cVector3 Cross(const cVector3& axis);
		void Normalize();

		inline types::f32 GetX() const { return _vec.x; }
		inline types::f32 GetY() const { return _vec.y; }
		inline types::f32 GetZ() const { return _vec.z; }
		inline void SetX(types::f32 value) { _vec.x = value; }
		inline void SetY(types::f32 value) { _vec.y = value; }
		inline void SetZ(types::f32 value) { _vec.z = value; }
		inline void AddX(types::f32 value) { _vec.x += value; }
		inline void AddY(types::f32 value) { _vec.y += value; }
		inline void AddZ(types::f32 value) { _vec.z += value; }
	};

	class cVector4
	{
		friend class cVector4;

		glm::vec4 _vec = glm::vec4(0.0f);

	public:
		explicit cVector4(const glm::vec4& vec);
		explicit cVector4(types::f32 value);
		explicit cVector4(types::f32 x, types::f32 y, types::f32 z, types::f32 w);
		~cVector4() = default;

		cVector4 operator+(const cVector4& vec) const;
		cVector4 operator-(const cVector4& vec) const;
		cVector4 operator*(const cVector4& vec) const;
		cVector4 operator/(const cVector4& vec) const;
		cVector4 operator+(types::f32 val) const;
		cVector4 operator-(types::f32 val) const;
		cVector4 operator*(types::f32 val) const;
		cVector4 operator/(types::f32 val) const;

		void Normalize();

		inline types::f32 GetX() const { return _vec.x; }
		inline types::f32 GetY() const { return _vec.y; }
		inline types::f32 GetZ() const { return _vec.z; }
		inline types::f32 GetW() const { return _vec.w; }
		inline void SetX(types::f32 value) { _vec.x = value; }
		inline void SetY(types::f32 value) { _vec.y = value; }
		inline void SetZ(types::f32 value) { _vec.z = value; }
		inline void SetW(types::f32 value) { _vec.w = value; }
		inline void AddX(types::f32 value) { _vec.x += value; }
		inline void AddY(types::f32 value) { _vec.y += value; }
		inline void AddZ(types::f32 value) { _vec.z += value; }
		inline void AddW(types::f32 value) { _vec.w += value; }
	};

	class cQuaternion
	{
		friend class cQuaternion;
	public: // TODO: remove this
		glm::quat _quat = {};

	public:
		explicit cQuaternion();
		explicit cQuaternion(const glm::quat& quat);
		explicit cQuaternion(types::f32 angle, const cVector3& axis);
		explicit cQuaternion(types::f32 w, types::f32 x, types::f32 y, types::f32 z);
		~cQuaternion() = default;

		cVector3 operator*(const cVector3& vec) const;
		cQuaternion operator*(const cQuaternion& quat) const;

		cVector3 EulerAngles() const;

		inline types::f32 GetX() const { return _quat.x; }
		inline types::f32 GetY() const { return _quat.y; }
		inline types::f32 GetZ() const { return _quat.z; }
		inline types::f32 GetW() const { return _quat.w; }
	};

	class cMatrix4
	{
		friend class cMatrix4;
		friend class cTransform;

	public: // TODO: uncomment this
		glm::mat4 _mat = {};

	public:
		explicit cMatrix4();
		explicit cMatrix4(const glm::mat4& mat);
		explicit cMatrix4(types::f32 value);
		explicit cMatrix4(const cVector3& position, const cVector3& direction, const cVector3& up);
		explicit cMatrix4(types::f32 fov, types::f32 aspect, types::f32 zNear, types::f32 zFar);
		~cMatrix4() = default;

		cMatrix4 operator*(const cMatrix4& mat) const;

		inline const glm::mat4& Get() const
		{
			return _mat;
		}
	};

	class cTransform
	{
		mutable cVector3 _position = cVector3(0.0f);
		mutable cVector3 _rotation = cVector3(0.0f);
		mutable cVector3 _scale = cVector3(0.0f);
		mutable cMatrix4 _world = cMatrix4(1.0f);

	public:
		explicit cTransform() = default;
		~cTransform() = default;

		void Transform();

		inline cVector3& GetPosition() const { return _position; }
		inline cVector3& GetRotation() const { return _rotation; }
		inline cVector3& GetScale() const { return _scale; }
		inline cMatrix4& GetWorld() const { return _world; }
		inline void SetPosition(const cVector3& position) { _position = position; }
		inline void SetRotation(const cVector3& rotation) { _rotation = rotation; }
		inline void SetScale(const cVector3& scale) { _scale = scale; }
	};

	class cMath : public iObject
	{
		TRITON_OBJECT(cMath)

		enum class EEulerAngle
		{
			PITCH,
			YAW,
			ROLL
		};

		static types::qword HashBytes(const types::u8* data, types::usize dataByteSize, types::qword mask);

	public:
		explicit cMath(cContext* context);
		virtual ~cMath() override final = default;

		static types::f32 DegreesToRadians(types::f32 degrees);
		static types::qword MakeHashMask(types::usize size);

		template <typename TValue>
		static types::qword Hash(const TValue& value, types::qword mask);

		static cMatrix4 Transform(
			cVector3 worldPosition,
			cVector3 worldRotation,
			cVector3 scale
		);
	};
}

template <typename TValue>
types::qword triton::cMath::Hash(const TValue& value, types::qword mask)
{
	if constexpr (std::is_same_v<TValue, cTag>)
	{
		return HashBytes((const types::u8*)value.GetData(), value.GetByteSize(), mask);
	}
	else if constexpr (std::is_same_v<TValue, std::string>)
	{
		return HashBytes((const types::u8*)value.c_str(), value.size(), mask);
	}
	else
	{
		return HashBytes((const types::u8*)&value, sizeof(TValue), mask);
	}
}
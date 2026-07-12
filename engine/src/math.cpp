// math.cpp

#include <cstring>
#include "math.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#define LZCNT32(x) __lzcnt(x)
#else
#include <immintrin.h>
#define LZCNT32(x) __builtin_clz(x)
#endif

using namespace types;

triton::cVector2::cVector2(const glm::vec2& vec) : _vec(vec) {}

triton::cVector2::cVector2(f32 value) : _vec(glm::vec2(value, value)) {}

triton::cVector2::cVector2(f32 x, f32 y) : _vec(glm::vec2(x, y)) {}

triton::cVector2 triton::cVector2::operator+(const cVector2& vec) const
{
	return cVector2(_vec + vec._vec);
}

triton::cVector2 triton::cVector2::operator-(const cVector2& vec) const
{
	return cVector2(_vec - vec._vec);
}

triton::cVector2 triton::cVector2::operator*(const cVector2& vec) const
{
	return cVector2(_vec * vec._vec);
}

triton::cVector2 triton::cVector2::operator/(const cVector2& vec) const
{
	return cVector2(_vec / vec._vec);
}

triton::cVector2 triton::cVector2::operator+(f32 val) const
{
	return cVector2(_vec + val);
}

triton::cVector2 triton::cVector2::operator-(f32 val) const
{
	return cVector2(_vec - val);
}

triton::cVector2 triton::cVector2::operator*(f32 val) const
{
	return cVector2(_vec * val);
}

triton::cVector2 triton::cVector2::operator/(f32 val) const
{
	return cVector2(_vec / val);
}

triton::cVector3::cVector3(const glm::vec3& vec) : _vec(vec) {}

triton::cVector3::cVector3(f32 value) : _vec(glm::vec3(value, value, value)) {}

triton::cVector3::cVector3(f32 x, f32 y, f32 z) : _vec(glm::vec3(x, y, z)) {}

triton::cVector3 triton::cVector3::operator+(const cVector3& vec) const
{
	return cVector3(_vec + vec._vec);
}

triton::cVector3 triton::cVector3::operator-(const cVector3& vec) const
{
	return cVector3(_vec - vec._vec);
}

triton::cVector3 triton::cVector3::operator*(const cVector3& vec) const
{
	return cVector3(_vec * vec._vec);
}

triton::cVector3 triton::cVector3::operator/(const cVector3& vec) const
{
	return cVector3(_vec / vec._vec);
}

triton::cVector3 triton::cVector3::operator+(f32 val) const
{
	return cVector3(_vec + val);
}

triton::cVector3 triton::cVector3::operator-(f32 val) const
{
	return cVector3(_vec - val);
}

triton::cVector3 triton::cVector3::operator*(f32 val) const
{
	return cVector3(_vec * val);
}

triton::cVector3 triton::cVector3::operator/(f32 val) const
{
	return cVector3(_vec / val);
}

triton::cVector3 triton::cVector3::Cross(const cVector3& axis)
{
	glm::vec3 vec = glm::cross(_vec, axis._vec);

	return cVector3(vec);
}

void triton::cVector3::Normalize()
{
	_vec = glm::normalize(_vec);
}

triton::cVector4::cVector4(const glm::vec4& vec) : _vec(vec) {}

triton::cVector4::cVector4(f32 value) : _vec(glm::vec4(value, value, value, value)) {}

triton::cVector4::cVector4(f32 x, f32 y, f32 z, f32 w) : _vec(glm::vec4(x, y, z, w)) {}

triton::cVector4 triton::cVector4::operator+(const cVector4& vec) const
{
	return cVector4(_vec + vec._vec);
}

triton::cVector4 triton::cVector4::operator-(const cVector4& vec) const
{
	return cVector4(_vec - vec._vec);
}

triton::cVector4 triton::cVector4::operator*(const cVector4& vec) const
{
	return cVector4(_vec * vec._vec);
}

triton::cVector4 triton::cVector4::operator/(const cVector4& vec) const
{
	return cVector4(_vec / vec._vec);
}

triton::cVector4 triton::cVector4::operator+(f32 val) const
{
	return cVector4(_vec + val);
}

triton::cVector4 triton::cVector4::operator-(f32 val) const
{
	return cVector4(_vec - val);
}

triton::cVector4 triton::cVector4::operator*(f32 val) const
{
	return cVector4(_vec * val);
}

triton::cVector4 triton::cVector4::operator/(f32 val) const
{
	return cVector4(_vec / val);
}

void triton::cVector4::Normalize()
{
	_vec = glm::normalize(_vec);
}

triton::cQuaternion::cQuaternion() : _quat(glm::identity<glm::quat>()) {}

triton::cQuaternion::cQuaternion(const glm::quat& quat) : _quat(quat) {}

triton::cQuaternion::cQuaternion(f32 angle, const cVector3& axis) : _quat(glm::angleAxis(angle, axis._vec)) {}

triton::cQuaternion::cQuaternion(types::f32 w, types::f32 x, types::f32 y, types::f32 z)
	: _quat(glm::quat(w, x, y, z)) {}

triton::cVector3 triton::cQuaternion::operator*(const cVector3& vec) const
{
	return cVector3(_quat * vec._vec);
}

triton::cQuaternion triton::cQuaternion::operator*(const cQuaternion& quat) const
{
	return cQuaternion(_quat * quat._quat);
}

triton::cVector3 triton::cQuaternion::EulerAngles() const
{
	return cVector3(glm::eulerAngles(_quat));
}

triton::cMatrix4::cMatrix4(const glm::mat4& mat) : _mat(mat) {}

triton::cMatrix4::cMatrix4(f32 value) : _mat(glm::mat4(value)) {}

triton::cMatrix4::cMatrix4(const cVector3& position, const cVector3& direction, const cVector3& up)
	: _mat(glm::lookAtRH(position._vec, position._vec + direction._vec, up._vec)) {}

triton::cMatrix4::cMatrix4(f32 fov, f32 aspect, f32 zNear, f32 zFar)
	: _mat(glm::perspective(cMath::DegreesToRadians(fov), aspect, zNear, zFar)) {}

triton::cMatrix4 triton::cMatrix4::operator*(const cMatrix4& mat) const
{
	return cMatrix4(_mat * mat._mat);
}

void triton::cTransform::Transform()
{
	const glm::quat quatX = glm::angleAxis(_rotation.GetX(), glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::quat quatY = glm::angleAxis(_rotation.GetY(), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::quat quatZ = glm::angleAxis(_rotation.GetZ(), glm::vec3(0.0f, 0.0f, 1.0f));
	_world._mat = glm::translate(glm::mat4(1.0f), _position._vec) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(glm::mat4(1.0f), _scale._vec);
}

triton::cMath::cMath(cContext* context) : iObject(context) {}

f32 triton::cMath::DegreesToRadians(f32 degrees)
{
	return glm::radians(degrees);
}

qword triton::cMath::MakeHashMask(usize size)
{
	unsigned int count = LZCNT32((unsigned int)size);
	qword mask = (qword)((1 << (31 - count)) - 1);

	return mask;
}

qword triton::cMath::HashBytes(const u8* data, usize dataByteSize, qword mask)
{
	qword hash = 0x9e3779b97f4a7c15ull;
	while (dataByteSize >= 4)
	{
		hash = (hash ^ (((qword)(*data++) * 0x9e3779b9ull) >> 32)) * 0xbf58476d1ce4e5b9ull;
		dataByteSize -= 4;
	}

	qword tail = 0;
	memcpy(&tail, data, dataByteSize);
	hash ^= tail;

	return hash & mask;
}
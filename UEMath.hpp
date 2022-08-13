#include "ModuleUtils.hpp"

#define FLOAT_NON_FRACTIONAL (8388608.f)
#define UEM_PI       3.1415

class Vector2
{
public:
	float x;
	float y;

	Vector2() : x(0.f), y(0.f)
	{

	}

	Vector2(float _x, float _y) : x(_x), y(_y)
	{

	}

	~Vector2()
	{

	}

	inline bool Invalid()
	{
		return x == 0 && y == 0;
	}
};

class Vector3
{
public:
	float x, y, z;

	inline Vector3() 
	{
		x = y = z = 0.0f;
	}

	inline Vector3(float X, float Y, float Z) 
	{
		x = X; y = Y; z = Z;
	}

	inline float operator[](int i) const 
	{
		return ((float*)this)[i];
	}

	inline Vector3& operator+=(float v) 
	{
		x += v; y += v; z += v; 
		return *this;
	}

	inline Vector3& operator-=(float v) 
	{
		x -= v; y -= v; z -= v; 
		return *this;
	}

	inline Vector3& operator-=(const Vector3& v) 
	{
		x -= v.x; y -= v.y; z -= v.z; 
		return *this;
	}

	inline Vector3 operator*(float v) const 
	{
		return Vector3(x * v, y * v, z * v);
	}

	inline Vector3 operator/(float v) const {
		return Vector3(x / v, y / v, z / v)
			;
	}

	inline Vector3& operator+=(const Vector3& v) 
	{
		x += v.x; y += v.y; z += v.z; 
		return *this;
	}

	inline Vector3 operator-(const Vector3& v) const 
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	inline Vector3 operator+(const Vector3& v) const 
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	inline Vector3& operator/=(float v) 
	{
		x /= v; y /= v; z /= v; 
		return *this;
	}

	inline bool Zero() const 
	{
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f);
	}

	inline bool Invalid()
	{
		return x == 0 && y == 0;
	}

	inline bool ContainsNaN() const
	{
		return (!CRT::m_isnan(x) ||
			!CRT::m_isnan(y)); /* ||
			!CRT::m_isnan(z));*/
	}

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Length()
	{
		return CRT::m_sqrtf(x * x + y * y + z * z);
	}

	inline float Distance(const Vector3& Other) const
	{
		const Vector3& a = *this;

		float dx = (a.x - Other.x);
		float dy = (a.y - Other.y);
		float dz = (a.z - Other.z);

		return CRT::m_sqrtf((dx * dx) + (dy * dy) + (dz * dz));
	}
};

class Matrix4
{
public:
	union
	{
		struct
		{
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};

	Matrix4()
	{

	}

	~Matrix4()
	{

	}
};
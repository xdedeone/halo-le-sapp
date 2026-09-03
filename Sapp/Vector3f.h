#pragma once

#include <cmath>

#pragma pack(push, 1)

class vect3f
{
public:
	float x, y, z;

	vect3f() : x(0), y(0), z(0) {}
    vect3f(float sx, float sy, float sz) : x(sx), y(sy), z(sz) {}
	
	vect3f operator+(const vect3f &param) const
	{
		return vect3f(x + param.x, y + param.y, z + param.z);
	}
    vect3f operator-(const vect3f &param) const
    {
       return vect3f(x - param.x, y - param.y, z - param.z);
    }
	vect3f operator*(const vect3f &param) const
	{
		return vect3f(x * param.x, y * param.y, z * param.z);
	}
	vect3f operator/(const vect3f &param) const
	{
		return vect3f(x / param.x, y / param.y, z / param.z);
	}

	vect3f operator+(float n) const
	{
		return vect3f(x + n, y + n, z + n);
	}
	vect3f operator-(float n) const
	{
		return vect3f(x - n, y - n, z - n);
	}
	vect3f operator*(float n) const
	{
		return vect3f(x * n, y * n, z * n);
	}
	vect3f operator/(float n) const
	{
		return vect3f(x / n, y / n, z / n);
	}

	vect3f operator+(int n) const
	{
		return vect3f(x + n, y + n, z + n);
	}
	vect3f operator-(int n) const
	{
		return vect3f(x - n, y - n, z - n);
	}
	vect3f operator*(int n) const
	{
		return vect3f(x * n, y * n, z * n);
	}
	vect3f operator/(int n) const
	{
		return vect3f(x / n, y / n, z / n);
	}

    void operator+=(const vect3f &param)
    {
        x += param.x;
        y += param.y;
        z += param.z;
    }
	void operator-=(const vect3f &param)
	{
		x -= param.x;
		y -= param.y;
		z -= param.z;
	}
	void operator*=(const vect3f &param)
	{
		x *= param.x;
		y *= param.y;
		z *= param.z;
	}
	void operator/=(const vect3f &param)
	{
		x /= param.x;
		y /= param.y;
		z /= param.z;
	}

	void operator+=(float n)
	{
		x += n;
		y += n;
		z += n;
	}
	void operator-=(float n)
	{
		x -= n;
		y -= n;
		z -= n;
	}
    void operator*=(float n)
    {
        x *= n;
        y *= n;
        z *= n;
    }
    void operator/=(float n)
    {
        x /= n;
        y /= n;
        z /= n;
    }

	void operator+=(int n)
	{
		x += n;
		y += n;
		z += n;
	}
	void operator-=(int n)
	{
		x -= n;
		y -= n;
		z -= n;
	}
	void operator*=(int n)
	{
		x *= n;
		y *= n;
		z *= n;
	}
	void operator/=(int n)
	{
		x /= n;
		y /= n;
		z /= n;
	}

	vect3f Normalize() const
    {
        float length = Length();
		return vect3f(x / length, y / length, z / length);
    }

	void Abs()
	{
		x = abs(x);
		y = abs(y);
		z = abs(z);
	}

	float Length() const
	{
		return sqrt(x*x + y*y + z*z);
	}
};

#pragma pack(pop)
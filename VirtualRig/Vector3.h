// vector3

#ifndef __VECTOR3__
#define __VECTOR3__

#include <math.h>

class Vector3
{
public:
	double x, y, z;		// componenti del vettore

	Vector3()
	{
		x = 0; 
		y = 0; 
		z = 0;
	}

	Vector3(double _x, double _y, double _z)
	{ 
		x = _x; 
		y = _y; 
		z = _z;
	}

	inline double Length() const
	{
		return sqrt( x * x + y * y + z * z );
	}

	inline double SquaredLength() const
	{
		return x * x + y * y + z * z;
	}

	inline Vector3 & operator =( Vector3 const & v )
	{
		x = v.x; 
		y = v.y; 
		z = v.z;
		return *this;
	}

	inline Vector3 operator + ( Vector3 const & v) const
	{
		return Vector3( x + v.x, y + v.y, z + v.z );
	}

	inline Vector3 & operator += ( Vector3 const & v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	inline Vector3 & operator -= ( Vector3 const & v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	inline Vector3 operator - ( Vector3 const & v) const
	{
		return Vector3( x - v.x, y - v.y, z - v.z );
	}

	inline Vector3 operator * ( const double s ) const
	{
		return Vector3( x * s, y * s, z * s);
	}

	inline Vector3 operator / ( const double s ) const
	{
		return Vector3( x / s, y / s, z / s);
	}

	// division by a scalar (double)
	inline Vector3 & operator /= ( const double s)
	{
		x /= s;
		y /= s;
		z /= s;

		return *this;
	}

	// Cross product
	inline Vector3 operator ^ ( Vector3 const & p ) const
	{
		return Vector3
			(
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x
			);
	}

	// Dot product
	inline double operator * ( Vector3 const & p ) const
	{
		return x * p.x + y * p.y + z * p.z;
	}

	inline Vector3 Normalize()
	{
		double length = Length();

		x /= length;
		y /= length;
		z /= length;

		return *this;
	}
	inline bool operator<(Vector3 const & p) const {

		return (this->y !=p.y)?(this->y <p.y):
			(this->x !=p.x)?(this->x < p.x):
			(this->z <p.z);
	}
};


#endif //__VECTOR3__
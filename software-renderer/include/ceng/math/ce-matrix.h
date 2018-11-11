/*****************************************************************************
*
* ce-matrix.h
*
* By Jari Korkala 9/2011
*
*
*****************************************************************************/

#ifndef _CONCEPT2_MATRIX_H
#define _CONCEPT2_MATRIX_H

#include "ce-vector.h"

namespace Ceng
{

class Matrix4
{
public:
	FLOAT32 data[4][4];

public:
	Matrix4();
	Matrix4(const Matrix4 &other);

	~Matrix4();

	Matrix4& operator = (const Matrix4 &other);
	Matrix4& operator += (const Matrix4 &other);
	Matrix4& operator -= (const Matrix4 &other);

	Matrix4& operator *= (const Matrix4 &other);
	Matrix4& operator *= (const FLOAT32 value);

	const Matrix4 operator + (const Matrix4 &other) const;
	const Matrix4 operator - (const Matrix4 &other) const;

	const Matrix4 operator * (const Matrix4 &other) const;
	const VectorF4 operator * (const VectorF4 &other) const;

	const Matrix4 operator * (const FLOAT32 value) const;

	void Identity();
};

//*************************************************************
// Inlined methods

inline Matrix4::Matrix4()
{
}

inline Matrix4::Matrix4(const Matrix4 &other)
{
	int k,j;

	for(k=0;k<4;k++)
	{
		for(j=0;j<4;j++)
		{
			data[k][j] = other.data[k][j];
		}
	}
}

inline Matrix4::~Matrix4()
{
}

inline void Matrix4::Identity()
{
	data[0][0] = 1.0;
	data[0][1] = 0;
	data[0][2] = 0;
	data[0][3] = 0;

	data[1][0] = 0;
	data[1][1] = 1.0;
	data[1][2] = 0;
	data[1][3] = 0;

	data[2][0] = 0;
	data[2][1] = 0;
	data[2][2] = 1.0;
	data[2][3] = 0;

	data[3][0] = 0;
	data[3][1] = 0;
	data[3][2] = 0;
	data[3][3] = 1.0;
}

inline Matrix4& Matrix4::operator = (const Matrix4 &other)
{
	memcpy(&data[0][0],&other.data[0][0],16*sizeof(FLOAT32));
	return *this;
}

inline Matrix4& Matrix4::operator += (const Matrix4 &other)
{
	data[0][0] += other.data[0][0];
	data[0][1] += other.data[0][1];
	data[0][2] += other.data[0][2];
	data[0][3] += other.data[0][3];

	data[1][0] += other.data[1][0];
	data[1][1] += other.data[1][1];
	data[1][2] += other.data[1][2];
	data[1][3] += other.data[1][3];

	data[2][0] += other.data[2][0];
	data[2][1] += other.data[2][1];
	data[2][2] += other.data[2][2];
	data[2][3] += other.data[2][3];

	data[3][0] += other.data[3][0];
	data[3][1] += other.data[3][1];
	data[3][2] += other.data[3][2];
	data[3][3] += other.data[3][3];

	return *this;
}

inline Matrix4& Matrix4::operator -= (const Matrix4 &other)
{
	data[0][0] -= other.data[0][0];
	data[0][1] -= other.data[0][1];
	data[0][2] -= other.data[0][2];
	data[0][3] -= other.data[0][3];

	data[1][0] -= other.data[1][0];
	data[1][1] -= other.data[1][1];
	data[1][2] -= other.data[1][2];
	data[1][3] -= other.data[1][3];

	data[2][0] -= other.data[2][0];
	data[2][1] -= other.data[2][1];
	data[2][2] -= other.data[2][2];
	data[2][3] -= other.data[2][3];

	data[3][0] -= other.data[3][0];
	data[3][1] -= other.data[3][1];
	data[3][2] -= other.data[3][2];
	data[3][3] -= other.data[3][3];

	return *this;
}

inline const Matrix4 Matrix4::operator + (const Matrix4 &other) const
{
	Matrix4 temp;

	temp.data[0][0] = data[0][0] + other.data[0][0];
	temp.data[0][1] = data[0][1] + other.data[0][1];
	temp.data[0][2] = data[0][2] + other.data[0][2];
	temp.data[0][3] = data[0][3] + other.data[0][3];

	temp.data[1][0] = data[1][0] + other.data[1][0];
	temp.data[1][1] = data[1][1] + other.data[1][1];
	temp.data[1][2] = data[1][2] + other.data[1][2];
	temp.data[1][3] = data[1][3] + other.data[1][3];

	temp.data[2][0] = data[2][0] + other.data[2][0];
	temp.data[2][1] = data[2][1] + other.data[2][1];
	temp.data[2][2] = data[2][2] + other.data[2][2];
	temp.data[2][3] = data[2][3] + other.data[2][3];

	temp.data[3][0] = data[3][0] + other.data[3][0];
	temp.data[3][1] = data[3][1] + other.data[3][1];
	temp.data[3][2] = data[3][2] + other.data[3][2];
	temp.data[3][3] = data[3][3] + other.data[3][3];

	return temp;
}

inline const Matrix4 Matrix4::operator - (const Matrix4 &other) const
{
	Matrix4 temp;

	temp.data[0][0] = data[0][0] - other.data[0][0];
	temp.data[0][1] = data[0][1] - other.data[0][1];
	temp.data[0][2] = data[0][2] - other.data[0][2];
	temp.data[0][3] = data[0][3] - other.data[0][3];

	temp.data[1][0] = data[1][0] - other.data[1][0];
	temp.data[1][1] = data[1][1] - other.data[1][1];
	temp.data[1][2] = data[1][2] - other.data[1][2];
	temp.data[1][3] = data[1][3] - other.data[1][3];

	temp.data[2][0] = data[2][0] - other.data[2][0];
	temp.data[2][1] = data[2][1] - other.data[2][1];
	temp.data[2][2] = data[2][2] - other.data[2][2];
	temp.data[2][3] = data[2][3] - other.data[2][3];

	temp.data[3][0] = data[3][0] - other.data[3][0];
	temp.data[3][1] = data[3][1] - other.data[3][1];
	temp.data[3][2] = data[3][2] - other.data[3][2];
	temp.data[3][3] = data[3][3] - other.data[3][3];

	return temp;
}

inline Matrix4& Matrix4::operator *= (const FLOAT32 value)
{
	data[0][0] *= value;
	data[0][1] *= value;
	data[0][2] *= value;
	data[0][3] *= value;

	data[1][0] *= value;
	data[1][1] *= value;
	data[1][2] *= value;
	data[1][3] *= value;

	data[2][0] *= value;
	data[2][1] *= value;
	data[2][2] *= value;
	data[2][3] *= value;

	data[3][0] *= value;
	data[3][1] *= value;
	data[3][2] *= value;
	data[3][3] *= value;

	return *this;
}

inline Matrix4& Matrix4::operator *= (const Matrix4 &other)
{
	// First row
	data[0][0] = data[0][0]*other.data[0][0] +
					data[0][1]*other.data[1][0] +
					data[0][2]*other.data[2][0] +
					data[0][3]*other.data[3][0];

	data[0][1] = data[0][0]*other.data[0][1] +
					data[0][1]*other.data[1][1] +
					data[0][2]*other.data[2][1] +
					data[0][3]*other.data[3][1];

	data[0][2] = data[0][0]*other.data[0][2] +
					data[0][1]*other.data[1][2] +
					data[0][2]*other.data[2][2] +
					data[0][3]*other.data[3][2];

	data[0][3] = data[0][0]*other.data[0][3] +
					data[0][1]*other.data[1][3] +
					data[0][2]*other.data[2][3] +
					data[0][3]*other.data[3][3];

	// Second row
	data[1][0] = data[1][0]*other.data[0][0] +
					data[1][1]*other.data[1][0] +
					data[1][2]*other.data[2][0] +
					data[1][3]*other.data[3][0];

	data[1][1] = data[1][0]*other.data[0][1] +
					data[1][1]*other.data[1][1] +
					data[1][2]*other.data[2][1] +
					data[1][3]*other.data[3][1];

	data[1][2] = data[1][0]*other.data[0][2] +
					data[1][1]*other.data[1][2] +
					data[1][2]*other.data[2][2] +
					data[1][3]*other.data[3][2];

	data[1][3] = data[1][0]*other.data[0][3] +
					data[1][1]*other.data[1][3] +
					data[1][2]*other.data[2][3] +
					data[1][3]*other.data[3][3];

	// Third row

	data[2][0] = data[2][0]*other.data[0][0] +
					data[2][1]*other.data[1][0] +
					data[2][2]*other.data[2][0] +
					data[2][3]*other.data[3][0];

	data[2][1] = data[2][0]*other.data[0][1] +
					data[2][1]*other.data[1][1] +
					data[2][2]*other.data[2][1] +
					data[2][3]*other.data[3][1];

	data[2][2] = data[2][0]*other.data[0][2] +
					data[2][1]*other.data[1][2] +
					data[2][2]*other.data[2][2] +
					data[2][3]*other.data[3][2];

	data[2][3] = data[2][0]*other.data[0][3] +
					data[2][1]*other.data[1][3] +
					data[2][2]*other.data[2][3] +
					data[2][3]*other.data[3][3];

	// Fourth row

	data[3][0] = data[3][0]*other.data[0][0] +
					data[3][1]*other.data[1][0] +
					data[3][2]*other.data[2][0] +
					data[3][3]*other.data[3][0];

	data[3][1] = data[3][0]*other.data[0][1] +
					data[3][1]*other.data[1][1] +
					data[3][2]*other.data[2][1] +
					data[3][3]*other.data[3][1];

	data[3][2] = data[3][0]*other.data[0][2] +
					data[3][1]*other.data[1][2] +
					data[3][2]*other.data[2][2] +
					data[3][3]*other.data[3][2];

	data[3][3] = data[3][0]*other.data[0][3] +
					data[3][1]*other.data[1][3] +
					data[3][2]*other.data[2][3] +
					data[3][3]*other.data[3][3];

	return *this;
}

inline const Matrix4 Matrix4::operator * (const Matrix4 &other) const
{
	Matrix4 temp;

	// First row
	temp.data[0][0] = data[0][0]*other.data[0][0] +
					data[0][1]*other.data[1][0] +
					data[0][2]*other.data[2][0] +
					data[0][3]*other.data[3][0];

	temp.data[0][1] = data[0][0]*other.data[0][1] +
					data[0][1]*other.data[1][1] +
					data[0][2]*other.data[2][1] +
					data[0][3]*other.data[3][1];

	temp.data[0][2] = data[0][0]*other.data[0][2] +
					data[0][1]*other.data[1][2] +
					data[0][2]*other.data[2][2] +
					data[0][3]*other.data[3][2];

	temp.data[0][3] = data[0][0]*other.data[0][3] +
					data[0][1]*other.data[1][3] +
					data[0][2]*other.data[2][3] +
					data[0][3]*other.data[3][3];

	// Second row
	temp.data[1][0] = data[1][0]*other.data[0][0] +
					data[1][1]*other.data[1][0] +
					data[1][2]*other.data[2][0] +
					data[1][3]*other.data[3][0];

	temp.data[1][1] = data[1][0]*other.data[0][1] +
					data[1][1]*other.data[1][1] +
					data[1][2]*other.data[2][1] +
					data[1][3]*other.data[3][1];

	temp.data[1][2] = data[1][0]*other.data[0][2] +
					data[1][1]*other.data[1][2] +
					data[1][2]*other.data[2][2] +
					data[1][3]*other.data[3][2];

	temp.data[1][3] = data[1][0]*other.data[0][3] +
					data[1][1]*other.data[1][3] +
					data[1][2]*other.data[2][3] +
					data[1][3]*other.data[3][3];

	// Third row

	temp.data[2][0] = data[2][0]*other.data[0][0] +
					data[2][1]*other.data[1][0] +
					data[2][2]*other.data[2][0] +
					data[2][3]*other.data[3][0];

	temp.data[2][1] = data[2][0]*other.data[0][1] +
					data[2][1]*other.data[1][1] +
					data[2][2]*other.data[2][1] +
					data[2][3]*other.data[3][1];

	temp.data[2][2] = data[2][0]*other.data[0][2] +
					data[2][1]*other.data[1][2] +
					data[2][2]*other.data[2][2] +
					data[2][3]*other.data[3][2];

	temp.data[2][3] = data[2][0]*other.data[0][3] +
					data[2][1]*other.data[1][3] +
					data[2][2]*other.data[2][3] +
					data[2][3]*other.data[3][3];

	// Fourth row

	temp.data[3][0] = data[3][0]*other.data[0][0] +
					data[3][1]*other.data[1][0] +
					data[3][2]*other.data[2][0] +
					data[3][3]*other.data[3][0];

	temp.data[3][1] = data[3][0]*other.data[0][1] +
					data[3][1]*other.data[1][1] +
					data[3][2]*other.data[2][1] +
					data[3][3]*other.data[3][1];

	temp.data[3][2] = data[3][0]*other.data[0][2] +
					data[3][1]*other.data[1][2] +
					data[3][2]*other.data[2][2] +
					data[3][3]*other.data[3][2];

	temp.data[3][3] = data[3][0]*other.data[0][3] +
					data[3][1]*other.data[1][3] +
					data[3][2]*other.data[2][3] +
					data[3][3]*other.data[3][3];

	return temp;
}

inline const VectorF4 Matrix4::operator *(const VectorF4 &other) const
{
	VectorF4 temp;

	temp.x = data[0][0]*other.x + data[0][1]*other.y +
				data[0][2]*other.z + data[0][3]*other.w;

	temp.y = data[1][0]*other.x + data[1][1]*other.y +
				data[1][2]*other.z + data[1][3]*other.w;

	temp.z = data[2][0]*other.x + data[2][1]*other.y +
				data[2][2]*other.z + data[2][3]*other.w;

	temp.w = data[3][0]*other.x + data[3][1]*other.y +
				data[3][2]*other.z + data[3][3]*other.w;

	return temp;
}

}

#endif
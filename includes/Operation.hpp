#pragma once
#include "Common.hpp"
#include "Matrix.hpp"

class Operation
{
public:
	// Matrix生成
	static Matrix identity();
	static Matrix translate(float x, float y, float z);
	static Matrix rotate(float ax, float ay, float az, float angleRadians);
	static Matrix perspective(float fovYRadians, float aspect, float nearPlane, float farPlane);

	// Quaternion演算
	static Quaternion quatFromAxisAngle(float ax, float ay, float az, float angleRadians);
	static Quaternion quatMultiply(const Quaternion& a, const Quaternion& b);
	static Quaternion quatNormalize(const Quaternion& q);
	static Matrix     quatToMatrix(const Quaternion& q);
};

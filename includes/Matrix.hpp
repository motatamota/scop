#pragma once

class Matrix
{
public:
	Matrix(); // 単位行列

	static Matrix identity();
	static Matrix translate(float x, float y, float z);
	static Matrix perspective(float fovYRadians, float aspect, float nearPlane, float farPlane);
	static Matrix lookAt(float eyeX, float eyeY, float eyeZ,
	                     float centerX, float centerY, float centerZ,
	                     float upX, float upY, float upZ);

	Matrix operator*(const Matrix& rhs) const;

	const float* data() const;

private:
	// OpenGL慣習の列優先格納: m[col * 4 + row]
	float m[16];
};

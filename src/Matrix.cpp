#include "Matrix.hpp"
#include <cmath>

Matrix::Matrix()
{
	// 単位行列
	for (int i = 0; i < 16; ++i)
		m[i] = 0.0f;
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

Matrix Matrix::identity()
{
	return Matrix();
}

Matrix Matrix::translate(float x, float y, float z)
{
	Matrix r;
	// 列優先で col=3 が平行移動列。m[12]=x, m[13]=y, m[14]=z
	r.m[12] = x;
	r.m[13] = y;
	r.m[14] = z;
	return r;
}

Matrix Matrix::perspective(float fovY, float aspect, float n, float f)
{
	Matrix r;
	// 一旦ゼロ行列化
	for (int i = 0; i < 16; ++i)
		r.m[i] = 0.0f;

	float tanHalf = std::tan(fovY / 2.0f);
	r.m[0]  = 1.0f / (aspect * tanHalf);
	r.m[5]  = 1.0f / tanHalf;
	r.m[10] = (f + n) / (n - f);
	r.m[11] = -1.0f;
	r.m[14] = (2.0f * f * n) / (n - f);
	// r.m[15] = 0; (パースペクティブ除算のため)
	return r;
}

Matrix Matrix::lookAt(float eyeX, float eyeY, float eyeZ,
                      float centerX, float centerY, float centerZ,
                      float upX, float upY, float upZ)
{
	// forward = normalize(center - eye)
	float fx = centerX - eyeX;
	float fy = centerY - eyeY;
	float fz = centerZ - eyeZ;
	float fLen = std::sqrt(fx * fx + fy * fy + fz * fz);
	fx /= fLen; fy /= fLen; fz /= fLen;

	// right = normalize(cross(forward, up))
	float rx = fy * upZ - fz * upY;
	float ry = fz * upX - fx * upZ;
	float rz = fx * upY - fy * upX;
	float rLen = std::sqrt(rx * rx + ry * ry + rz * rz);
	rx /= rLen; ry /= rLen; rz /= rLen;

	// up' = cross(right, forward)（直交化された上方向）
	float ux = ry * fz - rz * fy;
	float uy = rz * fx - rx * fz;
	float uz = rx * fy - ry * fx;

	Matrix m;
	for (int i = 0; i < 16; ++i)
		m.m[i] = 0.0f;

	// 列優先で格納: 列0=right, 列1=up', 列2=-forward, 列3=平行移動
	m.m[0]  = rx;   m.m[1]  = ux;   m.m[2]  = -fx;  m.m[3]  = 0.0f;
	m.m[4]  = ry;   m.m[5]  = uy;   m.m[6]  = -fy;  m.m[7]  = 0.0f;
	m.m[8]  = rz;   m.m[9]  = uz;   m.m[10] = -fz;  m.m[11] = 0.0f;
	m.m[12] = -(rx * eyeX + ry * eyeY + rz * eyeZ);
	m.m[13] = -(ux * eyeX + uy * eyeY + uz * eyeZ);
	m.m[14] = (fx * eyeX + fy * eyeY + fz * eyeZ);
	m.m[15] = 1.0f;

	return m;
}

Matrix Matrix::operator*(const Matrix& rhs) const
{
	Matrix result;
	for (int col = 0; col < 4; ++col)
	{
		for (int row = 0; row < 4; ++row)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
				sum += m[k * 4 + row] * rhs.m[col * 4 + k];
			result.m[col * 4 + row] = sum;
		}
	}
	return result;
}

const float* Matrix::data() const
{
	return m;
}

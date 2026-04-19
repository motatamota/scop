#include "Matrix.hpp"

Matrix::Matrix()
{
	for (int i = 0; i < 16; ++i)
		m[i] = 0.0f;
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
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

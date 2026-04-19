#pragma once

class Matrix
{
	friend class Operation;

public:
	Matrix(); // 単位行列

	Matrix operator*(const Matrix& rhs) const;
	const float* data() const;

private:
	// OpenGL慣習の列優先格納: m[col * 4 + row]
	float m[16];
};

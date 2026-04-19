#pragma once

struct position_s
{
	float x{};
	float y{};
	float z{};

	position_s& operator+=(const position_s& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
};

inline position_s operator+(position_s lhs, const position_s& rhs)
{
	lhs += rhs;
	return lhs;
}

struct Quaternion
{
	// デフォルトは単位クォータニオン (1, 0, 0, 0) = 無回転
	float w{1.0f};
	float x{};
	float y{};
	float z{};
};

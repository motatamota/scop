#include "Camera.hpp"
#include "Operation.hpp"
#include <cmath>

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::lookAt(const position_s& target)
{
	// 目標方向 d = normalize(target - eye)
	float dx = target.x - object_pos.x;
	float dy = target.y - object_pos.y;
	float dz = target.z - object_pos.z;
	float len = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (len < 1e-6f)
		return;  // eye と target が重なる → 何もしない
	dx /= len; dy /= len; dz /= len;

	// デフォルト前方 f0 = (0, 0, -1) を d に回す最短弧回転
	// 回転軸 = f0 × d = (dy, -dx, 0)
	// cos(θ) = f0 · d = -dz
	float ax = dy;
	float ay = -dx;
	float az = 0.0f;
	float cosT = -dz;

	// 軸長さが0 = f0 と d が平行
	float axisLen = std::sqrt(ax * ax + ay * ay + az * az);
	if (axisLen < 1e-6f)
	{
		if (cosT > 0.0f)
			orientation = Quaternion{1.0f, 0.0f, 0.0f, 0.0f};  // 同方向 = 無回転
		else
			orientation = Quaternion{0.0f, 0.0f, 1.0f, 0.0f};  // 逆方向 = Y軸周り180°
		return;
	}

	// cosT を [-1,1] にクランプ(浮動小数誤差対策)
	if (cosT > 1.0f) cosT = 1.0f;
	if (cosT < -1.0f) cosT = -1.0f;
	float angle = std::acos(cosT);
	orientation = Operation::quatFromAxisAngle(ax, ay, az, angle);
}

Matrix Camera::getViewMatrix() const
{
	// view = カメラ配置の逆変換 = R⁻¹ · T(-eye)
	// 単位クォータニオンの逆 = 共役 (虚部の符号反転)
	Quaternion inv{orientation.w, -orientation.x, -orientation.y, -orientation.z};
	Matrix R = Operation::quatToMatrix(inv);
	Matrix T = Operation::translate(-object_pos.x, -object_pos.y, -object_pos.z);
	return R * T;
}

#include "Operation.hpp"
#include <cmath>

// =========== Matrix生成 ===========

Matrix Operation::identity()
{
	return Matrix();
}

Matrix Operation::translate(float x, float y, float z)
{
	// 平行移動行列:
	// | 1 0 0 x |
	// | 0 1 0 y |
	// | 0 0 1 z |
	// | 0 0 0 1 |
	// 列優先格納: col=3 の上3要素 m[12..14] に (x,y,z) を入れる
	Matrix r;
	r.m[12] = x;
	r.m[13] = y;
	r.m[14] = z;
	return r;
}

Matrix Operation::rotate(float ax, float ay, float az, float angleRadians)
{
	// クォータニオン経由で行列化（回転ロジックを一箇所に集約）
	Quaternion q = quatFromAxisAngle(ax, ay, az, angleRadians);
	return quatToMatrix(q);
}

Matrix Operation::perspective(float fovY, float aspect, float n, float f)
{
	// 透視投影行列 (右手系, NDC z ∈ [-1, 1]):
	// let t = tan(fovY/2)
	// | 1/(aspect·t)  0      0            0           |
	// | 0             1/t    0            0           |
	// | 0             0     (f+n)/(n-f)  (2fn)/(n-f)  |
	// | 0             0     -1            0           |
	// m[11]=-1 によりクリップ w = -z_view が得られ、パースペクティブ除算が働く
	Matrix r;
	for (int i = 0; i < 16; ++i)
		r.m[i] = 0.0f;

	float tanHalf = std::tan(fovY / 2.0f);
	r.m[0]  = 1.0f / (aspect * tanHalf);
	r.m[5]  = 1.0f / tanHalf;
	r.m[10] = (f + n) / (n - f);
	r.m[11] = -1.0f;
	r.m[14] = (2.0f * f * n) / (n - f);
	return r;
}

// =========== Quaternion演算 ===========

Quaternion Operation::quatFromAxisAngle(float ax, float ay, float az, float angleRadians)
{
	// 軸角 (axis, θ) → クォータニオン
	// q = (cos(θ/2), ax·sin(θ/2), ay·sin(θ/2), az·sin(θ/2))
	// 半角を使うのは v' = q·v·q⁻¹ で両側から掛けるため (合計でθ回転になる)

	// 軸を単位ベクトル化 (|q|=1 を保つため)
	float len = std::sqrt(ax * ax + ay * ay + az * az);
	if (len > 0.0f)
	{
		ax /= len; ay /= len; az /= len;
	}

	float half = angleRadians * 0.5f;
	float s = std::sin(half);

	Quaternion q;
	q.w = std::cos(half);
	q.x = ax * s;
	q.y = ay * s;
	q.z = az * s;
	return q;
}

Quaternion Operation::quatMultiply(const Quaternion& a, const Quaternion& b)
{
	// Hamilton積: a * b = (a.w·b.w - a⃗·b⃗,  a.w·b⃗ + b.w·a⃗ + a⃗ × b⃗)
	//   w =  a.w·b.w - a.x·b.x - a.y·b.y - a.z·b.z
	//   x =  a.w·b.x + a.x·b.w + a.y·b.z - a.z·b.y
	//   y =  a.w·b.y - a.x·b.z + a.y·b.w + a.z·b.x
	//   z =  a.w·b.z + a.x·b.y - a.y·b.x + a.z·b.w
	// 「b で回したあと a で回す」= 行列積と同じ左から合成の意味を持つ
	Quaternion r;
	r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	return r;
}

Quaternion Operation::quatNormalize(const Quaternion& q)
{
	// q / |q|,  |q| = sqrt(w² + x² + y² + z²)
	// 単位クォータニオン (|q|=1) は純粋な回転を表す。
	// 累積計算で誤差が乗って |q|≠1 になるとスケーリングが混入するため毎回正規化。
	float len = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	Quaternion r = q;
	if (len > 0.0f)
	{
		r.w /= len;
		r.x /= len;
		r.y /= len;
		r.z /= len;
	}
	return r;
}

Matrix Operation::quatToMatrix(const Quaternion& q)
{
	// クォータニオン (単位) → 回転行列。v·q·q⁻¹ を展開すると:
	// 論理 (行優先表記):
	// | 1-2(y²+z²)   2(xy-wz)   2(xz+wy)   0 |
	// | 2(xy+wz)    1-2(x²+z²)  2(yz-wx)   0 |
	// | 2(xz-wy)    2(yz+wx)   1-2(x²+y²)  0 |
	// | 0           0          0           1 |
	// これを列優先メモリに格納するため、非対角要素は転置された位置に書き込む。
	float w = q.w, x = q.x, y = q.y, z = q.z;
	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, xz = x * z, yz = y * z;
	float wx = w * x, wy = w * y, wz = w * z;

	Matrix m;
	// col 0 (論理1列目)
	m.m[0]  = 1.0f - 2.0f * (yy + zz);
	m.m[1]  = 2.0f * (xy + wz);
	m.m[2]  = 2.0f * (xz - wy);
	m.m[3]  = 0.0f;

	// col 1
	m.m[4]  = 2.0f * (xy - wz);
	m.m[5]  = 1.0f - 2.0f * (xx + zz);
	m.m[6]  = 2.0f * (yz + wx);
	m.m[7]  = 0.0f;

	// col 2
	m.m[8]  = 2.0f * (xz + wy);
	m.m[9]  = 2.0f * (yz - wx);
	m.m[10] = 1.0f - 2.0f * (xx + yy);
	m.m[11] = 0.0f;

	// col 3 (平行移動なし)
	m.m[12] = 0.0f;
	m.m[13] = 0.0f;
	m.m[14] = 0.0f;
	m.m[15] = 1.0f;

	return m;
}

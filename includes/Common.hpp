#pragma once

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
const float ROT_SPEED = 0.1f;  // ラジアン/フレーム (USE_VSYNC=true 時の基準)
const float COLOR_CHANGE_SPEED = 0.01f;  // 色変換速度 (同上)
const float ZOOM_SPEED_RATIO = 0.02f;    // 1フレームで camDist の何倍だけ前後するか

// V-Sync を使うかどうか。
//   true  : glfwSwapInterval(1) で同期し、毎フレーム固定量を進める
//   false : V-Sync 無効。fps が暴れるので dt 方式でフレーム独立に補正する
// WSLg 等で V-Sync が効かない場合は false にする。
constexpr bool USE_VSYNC = false;

// USE_VSYNC=false の時に sleep で抑える目標 fps
constexpr double TARGET_FPS = 60.0;

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

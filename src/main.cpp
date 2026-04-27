#include "Window.hpp"
#include "gl_loader.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Matrix.hpp"
#include "Operation.hpp"
#include "Object.hpp"
#include "Camera.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>


int main(int ac, char **av) {
	std::cout
		<< "=== scop controls ===\n"
		<< "  Arrows      : rotate (Left/Right=Y, Up/Down=X)\n"
		<< "  Q / E       : rotate Z\n"
		<< "  W / S       : translate Y (up/down)\n"
		<< "  A / D       : translate X (left/right)\n"
		<< "  R / F       : translate Z (forward/back)\n"
		<< "  SPACE       : reset position & rotation\n"
		<< "  C           : toggle color <-> texture (smooth)\n"
		<< "  P           : toggle auto-rotation\n"
		<< "  ESC         : quit\n"
		<< "=====================" << std::endl;

	// ウィンドウ生成（GLFW・OpenGLコンテキスト込み）
	Window window(SCR_WIDTH, SCR_HEIGHT, "scop");
	if (!window.isReady())
		return -1;

	// 3D描画のため深度テストを有効化
	glEnable(GL_DEPTH_TEST);

	// シェダー読み込み
	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

	// テクスチャ読み込み
	Texture tex1("resources/image/macaron.bmp");
	if (!tex1.isReady())
		return -1;

	// .obj ロード + GPU登録
	Object obj;
	if (ac == 1)
	{
		// if (!obj.loadFromFile("resources/42image/xyzrgb_dragon.obj"))
		if (!obj.loadFromFile("resources/42image/42.obj"))
			return -1;
	}
	else
	{
		if (!obj.loadFromFile(av[1]))
			return -1;
	}
	obj.setupGPU();

	// サンプラーとテクスチャユニットの対応付け（一度だけ）
	shader.use();
	shader.setInt("texture1", 0);

	// カメラ距離をオブジェクトサイズから自動計算
	// d = R / tan(fovY/2) で縦方向に収まる + 余裕2倍
	const float FOV_Y_RAD = 45.0f * 3.14159265f / 180.0f;
	const float radius = obj.getBoundingRadius();
	const float camDist = (radius > 0.0f)
		? radius / std::tan(FOV_Y_RAD * 0.5f) * 2.0f
		: 10.0f;

	Camera camera;
	camera += position_s{0.0f, 0.0f, camDist};
	camera.lookAt(position_s{0.0f, 0.0f, 0.0f});
	Matrix projection = Operation::perspective(
		FOV_Y_RAD,
		static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
		0.1f, camDist * 4.0f  // far plane もスケールに追従
	);
#ifdef DEBUG
	std::cout << "[Camera] auto-fit radius=" << radius
	          << " distance=" << camDist << std::endl;
#endif

	float blend = 0.0f; // 実際の補間値
	bool  mode = false;
	bool  paused = false;  // P で自動回転の一時停止
	bool  prevP = false;   // P の押下エッジ検出用

	// FPS 計測: 1秒ごとに平均 fps を出力
	double fpsLastTime = glfwGetTime();
	int    fpsFrames = 0;
	// dt 計測用: 前フレームの時刻
	double prevFrameTime = glfwGetTime();

	// ループ処理
	while(!window.shouldClose())
	{
		// FPS 計測
		++fpsFrames;
		double now = glfwGetTime();
		if (now - fpsLastTime >= 1.0)
		{
			double fps = fpsFrames / (now - fpsLastTime);
			std::cout << "[FPS] " << fps << std::endl;
			fpsFrames = 0;
			fpsLastTime = now;
		}

		// V-Sync 無効時はフレーム経過秒で速度を補正 (60fps 基準で従来挙動と一致)
		float dt = static_cast<float>(now - prevFrameTime);
		prevFrameTime = now;
		const float stepScale = USE_VSYNC ? 1.0f : dt * 60.0f;

		// P の押下エッジで自動回転トグル
		bool curP = window.isKeyPressed(GLFW_KEY_P);
		if (curP && !prevP)
			paused = !paused;
		prevP = curP;

		// 常に少しずつ回転 (一時停止中はスキップ)
		if (!paused)
			obj.rotate(0.0f, 1.0f, 0.0f, -ROT_SPEED * stepScale / 20.0f);

		if (window.isKeyPressed(GLFW_KEY_ESCAPE))
			window.setShouldClose(true);

		// 矢印キーで全オブジェクトを回転(左右=Y軸、上下=X軸)
		if (window.isKeyPressed(GLFW_KEY_LEFT))
			obj.rotate(0.0f, 1.0f, 0.0f, -ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_RIGHT))
			obj.rotate(0.0f, 1.0f, 0.0f,  ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_UP))
			obj.rotate(1.0f, 0.0f, 0.0f, -ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_DOWN))
			obj.rotate(1.0f, 0.0f, 0.0f,  ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_Q))
			obj.rotate(0.0f, 0.0f, 1.0f,  ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_E))
			obj.rotate(0.0f, 0.0f, 1.0f,  -ROT_SPEED * stepScale);
		// W/A/S/D/R/F でオブジェクトを3軸双方向に平行移動
		// W/S = Y軸(上/下), A/D = X軸(左/右), R/F = Z軸(手前/奥)
		const float moveStep = radius * MOVE_SPEED_RATIO * stepScale;
		if (window.isKeyPressed(GLFW_KEY_W))
			obj += position_s{0.0f,  -moveStep, 0.0f};
		if (window.isKeyPressed(GLFW_KEY_S))
			obj += position_s{0.0f, moveStep, 0.0f};
		if (window.isKeyPressed(GLFW_KEY_A))
			obj += position_s{moveStep, 0.0f, 0.0f};
		if (window.isKeyPressed(GLFW_KEY_D))
			obj += position_s{-moveStep, 0.0f, 0.0f};
		if (window.isKeyPressed(GLFW_KEY_R))
			obj += position_s{0.0f, 0.0f,  moveStep};
		if (window.isKeyPressed(GLFW_KEY_F))
			obj += position_s{0.0f, 0.0f, -moveStep};
		// SPACE で初期位置・初期回転に戻す
		if (window.isKeyPressed(GLFW_KEY_SPACE))
			obj.reset();
		// C で色/テクスチャ切替 (遷移中は無視)
		if (window.isKeyPressed(GLFW_KEY_C) && (blend == 0.0f || blend == 1.0f))
			mode = !mode;
		if (mode)
		{
			blend += COLOR_CHANGE_SPEED * stepScale;
			if (blend > 1.0f)
				blend = 1.0f;
		}
		else
		{
			blend -= COLOR_CHANGE_SPEED * stepScale;
			if (blend < 0.0f)
				blend = 0.0f;
		}

		// 現在値を目標に近づける (スムーズ遷移)
		const float SPEED = 0.05f;
		if (blend < blend)
			blend = std::min(blend, blend + SPEED);
		else if (blend > blend)
			blend = std::max(blend, blend - SPEED);

		// glClearColor(0.18f, 0.18f, 0.20f, 1.0f);
		// glClearColor(0.13f, 0.14f, 0.16f, 1.0f);  // 濃紺寄りのチャコール
		// glClearColor(0.50f, 0.50f, 0.50f, 1.0f);  // 18% graycard 相当
		glClearColor(0.14f, 0.13f, 0.12f, 1.0f);  // 炭のような温かい黒

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tex1.bind(0);

		shader.use();
		shader.setFloat("blend", blend); // 色情報付与
		Matrix model = obj.getModelMatrix();
		Matrix view = camera.getViewMatrix();
		shader.setMat4("model", model.data());
		shader.setMat4("view", view.data());
		shader.setMat4("projection", projection.data());
		obj.draw();

		window.swapBuffers(); // スリープ用(映像切り替えも)
		window.pollEvents(); // キー入力対応

		// V-Sync OFF 時は目標 fps まで sleep で待機 (無駄な高 fps を抑制)
		if (!USE_VSYNC)
		{
			const double frameTime = 1.0 / TARGET_FPS;
			double elapsed = glfwGetTime() - now;
			if (elapsed < frameTime)
				std::this_thread::sleep_for(
					std::chrono::duration<double>(frameTime - elapsed));
		}
	}

	// VAO/VBO は Object のデストラクタで解放される
	return 0;
}

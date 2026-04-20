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


int main(int ac, char **av) {
	// ウィンドウ生成（GLFW・OpenGLコンテキスト込み）
	Window window(SCR_WIDTH, SCR_HEIGHT, "scop");
	if (!window.isReady())
		return -1;

	// 3D描画のため深度テストを有効化
	glEnable(GL_DEPTH_TEST);

	// シェダー読み込み
	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

	// テクスチャ読み込み
	Texture tex1("resources/image/wall.bmp");
	if (!tex1.isReady())
		return -1;

	// .obj ロード + GPU登録
	Object cube;
	if (ac == 1)
	{
		if (!cube.loadFromFile("resources/42image/42.obj"))
			return -1;
	}
	else
	{
		if (!cube.loadFromFile(av[1]))
			return -1;
	}
	cube.setupGPU();

	// サンプラーとテクスチャユニットの対応付け（一度だけ）
	shader.use();
	shader.setInt("texture1", 0);

	// MVP行列
	Camera camera;
	camera += position_s{0.0f, 0.0f, 10.0f};
	camera.lookAt(position_s{0.0f, 0.0f, 0.0f});
	Matrix projection = Operation::perspective(
		45.0f * 3.14159265f / 180.0f,
		static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
		0.1f, 100.0f
	);

	float blend = 0.0f; // 実際の補間値
	bool  mode = false;

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
#ifdef DEBUG
			double fps = fpsFrames / (now - fpsLastTime);
			std::cout << "[FPS] " << fps << std::endl;
#endif
			fpsFrames = 0;
			fpsLastTime = now;
		}

		// V-Sync 無効時はフレーム経過秒で速度を補正 (60fps 基準で従来挙動と一致)
		float dt = static_cast<float>(now - prevFrameTime);
		prevFrameTime = now;
		const float stepScale = USE_VSYNC ? 1.0f : dt * 60.0f;

		if (window.isKeyPressed(GLFW_KEY_ESCAPE))
			window.setShouldClose(true);

		// 矢印キーで全オブジェクトを回転(左右=Y軸、上下=X軸)
		if (window.isKeyPressed(GLFW_KEY_LEFT))
			cube.rotate(0.0f, 1.0f, 0.0f, -ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_RIGHT))
			cube.rotate(0.0f, 1.0f, 0.0f,  ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_UP))
			cube.rotate(1.0f, 0.0f, 0.0f, -ROT_SPEED * stepScale);
		if (window.isKeyPressed(GLFW_KEY_DOWN))
			cube.rotate(1.0f, 0.0f, 0.0f,  ROT_SPEED * stepScale);
		// 色切り替え終了ならば変更
		if (window.isKeyPressed(GLFW_KEY_SPACE) && (blend == 0.0f || blend == 1.0f))
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

		glClearColor(0.18f, 0.18f, 0.20f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tex1.bind(0);

		shader.use();
		shader.setFloat("blend", blend); // 色情報付与
		Matrix model = cube.getModelMatrix();
		Matrix view = camera.getViewMatrix();
		shader.setMat4("model", model.data());
		shader.setMat4("view", view.data());
		shader.setMat4("projection", projection.data());
		cube.draw();

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

#include "Window.hpp"
#include "gl_loader.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Matrix.hpp"
#include "Operation.hpp"
#include "Object.hpp"
#include "Camera.hpp"
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
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
	Texture tex2("resources/image/awesomeface.bmp");
	if (!tex1.isReady() || !tex2.isReady())
		return -1;

	// .obj ロード + GPU登録
	Object cube;
	if (!cube.loadFromFile("resources/42image/teapot.obj"))
		return -1;
	cube.setupGPU();

	// サンプラーとテクスチャユニットの対応付け（一度だけ）
	shader.use();
	shader.setInt("texture1", 0);
	shader.setInt("texture2", 1);

	// MVP行列
	Camera camera;
	camera += position_s{0.0f, 1.5f, 10.0f};  // teapot が映るよう後退
	camera.lookAt(position_s{0.0f, 1.5f, 0.0f});
	Matrix projection = Operation::perspective(
		45.0f * 3.14159265f / 180.0f,
		static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
		0.1f, 100.0f
	);

	const float ROT_SPEED = 0.03f;  // ラジアン/フレーム

	// ループ処理
	while(!window.shouldClose())
	{
		if (window.isKeyPressed(GLFW_KEY_ESCAPE))
			window.setShouldClose(true);

		// 矢印キーで全オブジェクトを回転(左右=Y軸、上下=X軸)
		if (window.isKeyPressed(GLFW_KEY_LEFT))
			cube.rotate(0.0f, 1.0f, 0.0f, -ROT_SPEED);
		if (window.isKeyPressed(GLFW_KEY_RIGHT))
			cube.rotate(0.0f, 1.0f, 0.0f,  ROT_SPEED);
		if (window.isKeyPressed(GLFW_KEY_UP))
			cube.rotate(1.0f, 0.0f, 0.0f, -ROT_SPEED);
		if (window.isKeyPressed(GLFW_KEY_DOWN))
			cube.rotate(1.0f, 0.0f, 0.0f,  ROT_SPEED);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tex1.bind(0);
		tex2.bind(1);

		shader.use();
		Matrix model = cube.getModelMatrix();
		Matrix view = camera.getViewMatrix();
		shader.setMat4("model", model.data());
		shader.setMat4("view", view.data());
		shader.setMat4("projection", projection.data());
		cube.draw();

		window.swapBuffers();
		window.pollEvents();
	}

	// VAO/VBO は Object のデストラクタで解放される
	return 0;
}

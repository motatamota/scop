#pragma once

#include <GLFW/glfw3.h>

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();

	bool isReady() const;
	bool shouldClose() const;
	void swapBuffers() const;
	void pollEvents() const;
	bool isKeyPressed(int key) const;
	void setShouldClose(bool value) const;

	GLFWwindow* raw() const;

private:
	GLFWwindow* window;
	bool ready;

	static void framebufferSizeCallback(GLFWwindow* win, int width, int height);
};

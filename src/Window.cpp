#include "Window.hpp"
#include "Common.hpp"
#include "gl_loader.hpp"
#include <iostream>

Window::Window(int width, int height, const char* title)
	: window(nullptr), ready(false)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(USE_VSYNC ? 1 : 0); // V-Sync の有効化切替 (Common.hpp)
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!loadGLFunctions())
	{
		std::cerr << "Failed to load GL functions" << std::endl;
		glfwDestroyWindow(window);
		window = nullptr;
		glfwTerminate();
		return;
	}

	glViewport(0, 0, width, height);
	ready = true;
#ifdef DEBUG
	std::cout << "[Window] created " << width << "x" << height
	          << " title=\"" << title << "\" GL="
	          << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
#endif
}

Window::~Window()
{
#ifdef DEBUG
	std::cout << "[Window] destroying" << std::endl;
#endif
	if (window != nullptr)
		glfwDestroyWindow(window);
	glfwTerminate();
}

bool Window::isReady() const
{
	return ready;
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window);
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(window);
}

void Window::pollEvents() const
{
	glfwPollEvents();
}

bool Window::isKeyPressed(int key) const
{
	return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::setShouldClose(bool value) const
{
	glfwSetWindowShouldClose(window, value);
}

GLFWwindow* Window::raw() const
{
	return window;
}

void Window::framebufferSizeCallback(GLFWwindow* win, int width, int height)
{
	glViewport(0, 0, width, height);
	(void)win;
}

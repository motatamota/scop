#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode = readFile(vertexPath);
	std::string fragmentCode = readFile(fragmentPath);
	const char* vCode = vertexCode.c_str();
	const char* fCode = fragmentCode.c_str();

	unsigned int vertex = compileShader(GL_VERTEX_SHADER, vCode, "VERTEX");
	unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fCode, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkErrors(ID, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string& name, const float* value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

std::string Shader::readFile(const char* path) const
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "ERROR::SHADER::FILE_NOT_FOUND: " << path << std::endl;
		return "";
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

unsigned int Shader::compileShader(unsigned int type, const char* source, const char* typeName) const
{
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	checkErrors(shader, typeName);
	return shader;
}

void Shader::checkErrors(unsigned int id, const char* type) const
{
	int success;
	char infoLog[1024];
	if (std::string(type) != "PROGRAM")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(id, 1024, NULL, infoLog);
			std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 1024, NULL, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
	}
}

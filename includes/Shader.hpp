#pragma once

#include "gl_loader.hpp"
#include <string>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	void use() const;
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setMat4(const std::string& name, const float* value) const;

private:
	unsigned int ID;

	std::string readFile(const char* path) const;
	unsigned int compileShader(unsigned int type, const char* source, const char* typeName) const;
	void checkErrors(unsigned int id, const char* type) const;
};

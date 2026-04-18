#pragma once

#include "gl_loader.hpp"

class Texture
{
public:
	Texture(const char* path);
	~Texture();

	bool isReady() const;
	void bind(unsigned int unit = 0) const;
	unsigned int getID() const;

private:
	unsigned int ID;
	int width;
	int height;
	bool ready;
};

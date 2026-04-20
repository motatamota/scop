#include "Texture.hpp"
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <iostream>

static uint32_t readU32(const unsigned char* p)
{
	return static_cast<uint32_t>(p[0])
		| (static_cast<uint32_t>(p[1]) << 8)
		| (static_cast<uint32_t>(p[2]) << 16)
		| (static_cast<uint32_t>(p[3]) << 24);
}

static int32_t readI32(const unsigned char* p)
{
	return static_cast<int32_t>(readU32(p));
}

static uint16_t readU16(const unsigned char* p)
{
	return static_cast<uint16_t>(p[0])
		| (static_cast<uint16_t>(p[1]) << 8);
}

Texture::Texture(const char* path)
	: ID(0), width(0), height(0), ready(false)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "ERROR::TEXTURE::FILE_NOT_FOUND: " << path << std::endl;
		return;
	}

	// BITMAPFILEHEADER（14バイト）
	unsigned char fileHeader[14];
	file.read(reinterpret_cast<char*>(fileHeader), 14);
	if (!file || fileHeader[0] != 'B' || fileHeader[1] != 'M')
	{
		std::cerr << "ERROR::TEXTURE::NOT_A_BMP: " << path << std::endl;
		return;
	}
	uint32_t dataOffset = readU32(fileHeader + 10);

	// DIBヘッダサイズ（4バイト）→ その後にwidth/height/planes/bppが続く
	unsigned char dibSizeBytes[4];
	file.read(reinterpret_cast<char*>(dibSizeBytes), 4);
	uint32_t dibSize = readU32(dibSizeBytes);
	if (dibSize < 12)
	{
		std::cerr << "ERROR::TEXTURE::INVALID_DIB_SIZE: " << path << std::endl;
		return;
	}

	std::vector<unsigned char> dib(dibSize - 4);
	file.read(reinterpret_cast<char*>(dib.data()), dibSize - 4);
	if (!file)
	{
		std::cerr << "ERROR::TEXTURE::DIB_READ_FAILED: " << path << std::endl;
		return;
	}

	// dib[0..3] = width, dib[4..7] = height, dib[8..9] = planes, dib[10..11] = bpp
	int32_t w = readI32(dib.data());
	int32_t h = readI32(dib.data() + 4);
	uint16_t bpp = readU16(dib.data() + 10);

	if (w <= 0 || h == 0)
	{
		std::cerr << "ERROR::TEXTURE::INVALID_DIMENSIONS: " << path << std::endl;
		return;
	}
	if (bpp != 24 && bpp != 32)
	{
		std::cerr << "ERROR::TEXTURE::UNSUPPORTED_BPP (" << bpp << "): " << path << std::endl;
		return;
	}

	width = w;
	bool topDown = (h < 0);
	height = topDown ? -h : h;
	int channels = bpp / 8;

	// 行サイズは4バイト境界にパディングされる
	int rowSize = ((width * channels) + 3) & ~3;
	int rawSize = rowSize * height;

	file.seekg(dataOffset, std::ios::beg);
	std::vector<unsigned char> raw(rawSize);
	file.read(reinterpret_cast<char*>(raw.data()), rawSize);
	if (!file)
	{
		std::cerr << "ERROR::TEXTURE::PIXEL_READ_FAILED: " << path << std::endl;
		return;
	}

	// パディングを取り除き、BMPの行順（height>0なら下→上）を OpenGL 慣習（下→上）に合わせる
	std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * channels);
	for (int y = 0; y < height; ++y)
	{
		int srcRow = topDown ? (height - 1 - y) : y;
		std::memcpy(
			&pixels[static_cast<size_t>(y) * width * channels],
			&raw[static_cast<size_t>(srcRow) * rowSize],
			static_cast<size_t>(width) * channels
		);
	}

	GLenum format = (channels == 4) ? GL_BGRA : GL_BGR;
	GLenum internalFormat = (channels == 4) ? GL_RGBA : GL_RGB;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	ready = true;
#ifdef DEBUG
	std::cout << "[Texture] loaded " << path
	          << " ID=" << ID << " " << width << "x" << height
	          << " bpp=" << bpp << std::endl;
#endif
}

Texture::~Texture()
{
#ifdef DEBUG
	if (ID != 0)
		std::cout << "[Texture] destroying ID=" << ID << std::endl;
#endif
	if (ID != 0)
		glDeleteTextures(1, &ID);
}

bool Texture::isReady() const
{
	return ready;
}

void Texture::bind(unsigned int unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

unsigned int Texture::getID() const
{
	return ID;
}

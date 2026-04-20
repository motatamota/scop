#pragma once
#include <string>
#include <vector>
#include "Common.hpp"
#include "Matrix.hpp"

class Object
{
public:
	Object();
	virtual ~Object();

	Object& operator+=(const position_s& delta);
	void rotate(float axisX, float axisY, float axisZ, float angleRadians);

	Matrix getModelMatrix() const;

	// .obj をパースして mesh_data に書き込む。成功時 true
	bool loadFromFile(const std::string& path);
	// 頂点あたり8float (x,y,z,u,v,r,g,b) のインターリーブデータ
	const std::vector<float>& getMeshData() const;
	// getMeshData().size() / 8 と等価 (glDrawArrays 用)
	size_t getVertexCount() const;

	// scop 固有: 5段階グレーを三角形ごとに循環で割り当てる
	// (subject の "different faces visually distinguishable" 要件用)
	void applyGrayPaletteScop();

	// VAO/VBO を生成し mesh_data を GPU にアップロード(GLコンテキスト必須)
	void setupGPU();
	// VAO を bind して glDrawArrays を発行
	void draw() const;

	// GLリソースの二重解放を防ぐためコピー禁止
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;

protected:
	position_s object_pos{};
	Quaternion orientation{};
	std::vector<float> mesh_data{};  // x,y,z,u,v,r,g,b の繰り返し
	unsigned int VAO{0};
	unsigned int VBO{0};
};

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
	// 頂点あたり5float (x,y,z,u,v) のインターリーブデータ
	const std::vector<float>& getMeshData() const;
	// getMeshData().size() / 5 と等価 (glDrawArrays 用)
	size_t getVertexCount() const;

protected:
	position_s object_pos{};
	Quaternion orientation{};
	std::vector<float> mesh_data{};  // x,y,z,u,v の繰り返し
};

#include "Object.hpp"
#include "Operation.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Object::Object()
{
}

Object::~Object()
{
}

Object& Object::operator+=(const position_s& delta)
{
	object_pos += delta;
	return *this;
}

void Object::rotate(float axisX, float axisY, float axisZ, float angleRadians)
{
	// 新しい回転を既存の姿勢に合成（左から掛ける）
	Quaternion delta = Operation::quatFromAxisAngle(axisX, axisY, axisZ, angleRadians);
	orientation = Operation::quatMultiply(delta, orientation);
	// 誤差蓄積を避けるため正規化
	orientation = Operation::quatNormalize(orientation);
}

Matrix Object::getModelMatrix() const
{
	// model = translate(pos) * rotation(orientation)
	// 回転を先に適用して、その後位置に移動
	Matrix t = Operation::translate(object_pos.x, object_pos.y, object_pos.z);
	Matrix r = Operation::quatToMatrix(orientation);
	return t * r;
}

// f トークン ("123" / "123/45" / "123/45/67" / "123//67") から頂点インデックスだけ抜く
static int parseFaceIndex(const std::string& token, int vertCount)
{
	if (token.empty())
		return -1;
	size_t slash = token.find('/');
	std::string num = (slash == std::string::npos) ? token : token.substr(0, slash);
	int idx = 0;
	try { idx = std::stoi(num); }
	catch (...) { return -1; }
	// .obj は1始まり、負数は末尾からの相対
	if (idx < 0) idx = vertCount + idx + 1;
	return idx - 1;  // 0始まりに変換
}

bool Object::loadFromFile(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		std::cerr << "failed to open: " << path << std::endl;
		return false;
	}

	std::vector<position_s> verts;
	mesh_data.clear();

	// バウンディングボックス (UV正規化用)
	float minX = 0, maxX = 0, minY = 0, maxY = 0;
	bool first = true;

	std::string line;
	while (std::getline(ifs, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		std::istringstream iss(line);
		std::string tag;
		iss >> tag;

		if (tag == "v")
		{
			position_s p;
			iss >> p.x >> p.y >> p.z;
			verts.push_back(p);
			if (first)
			{
				minX = maxX = p.x;
				minY = maxY = p.y;
				first = false;
			}
			else
			{
				if (p.x < minX) minX = p.x;
				if (p.x > maxX) maxX = p.x;
				if (p.y < minY) minY = p.y;
				if (p.y > maxY) maxY = p.y;
			}
		}
		else if (tag == "f")
		{
			std::vector<int> idx;
			std::string tok;
			while (iss >> tok)
			{
				int i = parseFaceIndex(tok, static_cast<int>(verts.size()));
				if (i >= 0 && i < static_cast<int>(verts.size()))
					idx.push_back(i);
			}
			// ファン三角形化: (0,i,i+1) for i = 1..n-2
			for (size_t i = 1; i + 1 < idx.size(); ++i)
			{
				int tri[3] = { idx[0], idx[static_cast<int>(i)], idx[static_cast<int>(i) + 1] };
				for (int k = 0; k < 3; ++k)
				{
					const position_s& p = verts[tri[k]];
					mesh_data.push_back(p.x);
					mesh_data.push_back(p.y);
					mesh_data.push_back(p.z);
					// UVはあとで埋める(スパン確定後)
					mesh_data.push_back(0.0f);
					mesh_data.push_back(0.0f);
				}
			}
		}
		// v/vn/vt/mtllib/usemtl/o/g/s は無視
	}

	// UVをxy平面への正規化プロジェクションで後書き
	float dx = maxX - minX;
	float dy = maxY - minY;
	if (dx < 1e-6f) dx = 1.0f;
	if (dy < 1e-6f) dy = 1.0f;
	for (size_t i = 0; i + 4 < mesh_data.size(); i += 5)
	{
		float x = mesh_data[i];
		float y = mesh_data[i + 1];
		mesh_data[i + 3] = (x - minX) / dx;
		mesh_data[i + 4] = (y - minY) / dy;
	}

	std::cerr << "loaded: " << path
	          << " verts=" << verts.size()
	          << " triangles=" << (mesh_data.size() / 15) << std::endl;
	return true;
}

const std::vector<float>& Object::getMeshData() const
{
	return mesh_data;
}

size_t Object::getVertexCount() const
{
	return mesh_data.size() / 5;
}

#include "Object.hpp"
#include "Operation.hpp"
#include "gl_loader.hpp"
#include <cmath>
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
#ifdef DEBUG
	if (VBO || VAO)
		std::cout << "[Object] destroying VAO=" << VAO << " VBO=" << VBO << std::endl;
#endif
	if (VBO) glDeleteBuffers(1, &VBO);
	if (VAO) glDeleteVertexArrays(1, &VAO);
}

Object& Object::operator+=(const position_s& delta)
{
	object_pos += delta;
#ifdef DEBUG
	std::cout << "[Object] pos += (" << delta.x << "," << delta.y << "," << delta.z << ")"
	          << " → (" << object_pos.x << "," << object_pos.y << "," << object_pos.z << ")" << std::endl;
#endif
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
	float minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
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
				minZ = maxZ = p.z;
				first = false;
			}
			else
			{
				if (p.x < minX) minX = p.x;
				if (p.x > maxX) maxX = p.x;
				if (p.y < minY) minY = p.y;
				if (p.y > maxY) maxY = p.y;
				if (p.z < minZ) minZ = p.z;
				if (p.z > maxZ) maxZ = p.z;
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
					// 色はあとで applyGrayPaletteScop() で埋める
					mesh_data.push_back(0.0f);
					mesh_data.push_back(0.0f);
					mesh_data.push_back(0.0f);
				}
			}
		}

	}

	// 中央調整用
	float centerX = (minX + maxX) / 2.0f;
	float centerY = (minY + maxY) / 2.0f;
	float centerZ = (minZ + maxZ) / 2.0f;

	// UVをxy平面への正規化プロジェクションで後書き
	float dx = maxX - minX;
	float dy = maxY - minY;
	float dz = maxZ - minZ;
	if (dx < 1e-6f) dx = 1.0f;
	if (dy < 1e-6f) dy = 1.0f;

	// AABB を包む球の半径 (中心からコーナーまでの距離)
	bounding_radius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
	for (size_t i = 0; i + 7 < mesh_data.size(); i += 8)
	{
		mesh_data[i] = mesh_data[i] - centerX;
		mesh_data[i + 1] = mesh_data[i + 1] - centerY;
		mesh_data[i + 2] = mesh_data[i + 2] - centerZ;
		float x = mesh_data[i];
		float y = mesh_data[i + 1];
		mesh_data[i + 3] = (x - minX) / dx;
		mesh_data[i + 4] = (y - minY) / dy;
	}
	// scop 固有: 面ごとに 5段階グレーを循環割当
	applyGrayPaletteScop();
#ifdef DEBUG
	std::cout << "[Object] loadFromFile " << path
	          << " verts=" << verts.size()
	          << " triangles=" << (mesh_data.size() / 24) << std::endl;
#endif
	return true;
}

float Object::getBoundingRadius() const
{
	return bounding_radius;
}

void Object::applyGrayPaletteScop()
{
	// 5段階グレー (濃→淡)。subject の "subtle shades of gray" に合わせた中間トーン
	static const float GRAY_PALETTE_SCOP[5] = {
		0.30f, 0.45f, 0.60f, 0.75f, 0.90f
	};
	const size_t FLOATS_PER_VERTEX = 8;
	const size_t VERTICES_PER_TRI  = 3;
	const size_t triCount = mesh_data.size() / (FLOATS_PER_VERTEX * VERTICES_PER_TRI);
	for (size_t tri = 0; tri < triCount; ++tri)
	{
		float g = GRAY_PALETTE_SCOP[tri % 5];
		for (size_t k = 0; k < VERTICES_PER_TRI; ++k)
		{
			size_t base = (tri * VERTICES_PER_TRI + k) * FLOATS_PER_VERTEX;
			mesh_data[base + 5] = g;
			mesh_data[base + 6] = g;
			mesh_data[base + 7] = g;
		}
	}
#ifdef DEBUG
	std::cout << "[Object] applyGrayPaletteScop triangles=" << triCount << std::endl;
#endif
}

const std::vector<float>& Object::getMeshData() const
{
	return mesh_data;
}

size_t Object::getVertexCount() const
{
	return mesh_data.size() / 8;
}

void Object::setupGPU()
{
	if (mesh_data.empty())
	{
#ifdef DEBUG
		std::cout << "[Object] setupGPU skipped (mesh_data empty)" << std::endl;
#endif
		return;
	}
	if (VAO == 0) glGenVertexArrays(1, &VAO);
	if (VBO == 0) glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
	             static_cast<GLsizeiptr>(mesh_data.size() * sizeof(float)),
	             mesh_data.data(), GL_STATIC_DRAW);
	// position: location = 0 (vec3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texcoord: location = 1 (vec2)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
	                      (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// color (scop グレー): location = 2 (vec3)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
	                      (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#ifdef DEBUG
	std::cout << "[Object] setupGPU VAO=" << VAO << " VBO=" << VBO
	          << " vertices=" << getVertexCount()
	          << " bytes=" << (mesh_data.size() * sizeof(float)) << std::endl;
#endif
}

void Object::draw() const
{
	if (VAO == 0)
		return;
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(getVertexCount()));
}

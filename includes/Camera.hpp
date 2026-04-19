#pragma once
#include "Object.hpp"

class Camera : public Object
{
public:
	Camera();
	~Camera() override;

	// targetを注視するよう orientation を更新(最短弧回転)
	void lookAt(const position_s& target);

	Matrix getViewMatrix() const;
};

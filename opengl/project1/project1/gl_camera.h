#pragma once
#include <cmath>
#include "vec3.h"
#include "gl_matrix.h"

struct Camera {
	Vector3f origin;
	//look_at は origin->look_atを示す方向ベクトル
	//ここでは固定された注視点として扱わない．
	Vector3f lookat_d;
	Vector3f up;

	float sin70 = sin(70.0/360.0 * 2 * 3.14159265); //up_limit
	float tan70 = tan(70.0/360.0 * 2 * 3.14159265);

	GLfloat origin_sensitivity[3] = { 0.05,0.05,0.05 };
	GLfloat lookat_sensitivity[2] = { 0.001,0.001};
	
	Camera() {reset();}

	void reset() {
		//origin = Vector3f(0, 30, -100);
		origin = Vector3f(-30, 30, -100);
		lookat_d = Vector3f(1,0,1).Normalize();
		up = Vector3f(0, 1, 0).Normalize();
	}

	Matrix update(bool* key_response, float *mouse_offset) {
		
		int dx = 0, dy = 0, dz = 0;
		if (key_response[0]) { dz += 1; } //w front
		if (key_response[1]) { dx += 1; } //a left
		if (key_response[2]) { dz -= 1; } //s back
		if (key_response[3]) { dx -= 1; } //d right
		if (key_response[4]) { dy += 1; } //space up 
		if (key_response[5]) { dy -= 1; } //x dawn

		update_origin(dx, dy, dz);
		
		update_lookat_d(mouse_offset);


		if (key_response[6]) { reset(); }
		return Matrix::lookat(origin, origin+lookat_d, up);	
	}

private:
	//x y z must == (0 or -1 or 1)
	void update_origin(int dx, int dy, int dz) {
		//left right
		if (dx != 0) {
			float fdx = dx*origin_sensitivity[0];
			Vector3f Right = up.Cross(lookat_d);
			Right.Normalize();
			origin += Right*fdx;
		}

		//up down
		if (dy != 0) {
			float fdy = dy*origin_sensitivity[1];
			origin += up*fdy;
		}

		//flont back
		if (dz != 0) {
			float fdz = dz*origin_sensitivity[2];
			auto d = lookat_d.Normalize();
			origin += d*fdz;
		}	
	}

	//mdx, mdy : pixel
	void update_lookat_d(float* mo) {
		float m_dx = mo[0], m_dy = mo[1];

		if (m_dx != 0) {
			float fdx = m_dx*lookat_sensitivity[0];

			Vector3f left = lookat_d.Cross(up);
			left.Normalize();
			lookat_d += left*fdx;
			lookat_d.Normalize();
		}

		if (m_dy != 0) {
			float fdy = m_dy*lookat_sensitivity[1];
			Vector3f left = lookat_d.Cross(up);
			Vector3f tup = lookat_d.Cross(left);
			lookat_d += tup * fdy;
			lookat_d.Normalize();
		}
		
		//limit camera y angle
		if (lookat_d[1] > sin70) {
			float lxy = sqrtf(lookat_d[0] * lookat_d[0] + lookat_d[2] * lookat_d[2]);
			lookat_d[1] = lxy*tan70;
			lookat_d.Normalize();
		}
		if (-sin70 > lookat_d[1]) {
			float lxy = sqrtf(lookat_d[0] * lookat_d[0] + lookat_d[2] * lookat_d[2]);
			lookat_d[1] = -lxy*tan70;
			lookat_d.Normalize();
		}
	}	
};

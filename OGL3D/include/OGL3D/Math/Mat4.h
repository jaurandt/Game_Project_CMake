#pragma once

#include <OGL3D/Prerequisites.h>
#include <OGL3D/Math/Vec4.h>
#include <cmath>

class Mat4
{
public:
	Mat4()
	{
		setIdentity();
	}

	void setIdentity()
	{
		::memset(m_values, 0, sizeof(float) * 16);
		for (int i = 0; i < 4; ++i)
		{
			m_values[i][i] = 1.0f;
		}
	}

	void setScale(const Vec4& scale)
	{
		m_values[0][0] = scale.x;
		m_values[1][1] = scale.y;
		m_values[2][2] = scale.z;
	}

	void setTranslation(const Vec4& translation)
	{
		m_values[3][0] = translation.x;
		m_values[3][1] = translation.y;
		m_values[3][2] = translation.z;
	}

	void setRotationX(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		m_values[1][1] = c;
		m_values[1][2] = s;
		m_values[2][1] = -s;
		m_values[2][2] = c;
	}

	void setRotationY(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		m_values[0][0] = c;
		m_values[0][2] = -s;
		m_values[2][0] = s;
		m_values[2][2] = c;
	}

	void setRotationZ(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		m_values[0][0] = c;
		m_values[0][1] = s;
		m_values[1][0] = -s;
		m_values[1][1] = c;
	}

	void operator *=(const Mat4& other)
	{
		Mat4 result;
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				result.m_values[row][col] = 0.0f;
				for (int k = 0; k < 4; ++k)
				{
					result.m_values[row][col] += m_values[row][k] * other.m_values[k][col];
				}
			}
		}
		::memcpy(m_values, result.m_values, sizeof(float) * 16);
	}

	// Build a perspective projection (row-major layout)
	// fovY in radians, aspect = width / height, znear > 0, zfar > znear
	void setPerspective(float fovY, float aspect, float nearZ, float farZ)
	{
		float f = 1.0f / tanf(fovY * 0.5f);
		::memset(m_values, 0, sizeof(float) * 16);
		m_values[0][0] = f / aspect;
		m_values[1][1] = f;
		m_values[2][2] = (farZ + nearZ) / (nearZ - farZ);
		m_values[2][3] = (2.0f * farZ * nearZ) / (nearZ - farZ);
		m_values[3][2] = -1.0f;
	}

	void setLookAt(const Vec4& eye, const Vec4& center, const Vec4& up)
	{
		// forward = normalize(center - eye)
		float fx = center.x - eye.x;
		float fy = center.y - eye.y;
		float fz = center.z - eye.z;
		float fLen = sqrtf(fx * fx + fy * fy + fz * fz);
		if (fLen > 1e-6f) { fx /= fLen; fy /= fLen; fz /= fLen; }

		// right = normalize(cross(forward, up))
		float rx = fy * up.z - fz * up.y;
		float ry = fz * up.x - fx * up.z;
		float rz = fx * up.y - fy * up.x;
		float rLen = sqrtf(rx * rx + ry * ry + rz * rz);
		if (rLen > 1e-6f) { rx /= rLen; ry /= rLen; rz /= rLen; }
		
		// up' = cross(right, forward)
		float ux = ry * fz - rz * fy;
		float uy = rz * fx - rx * fz;
		float uz = rx * fy - ry * fx;

		setIdentity();

		// Row-major lookAt matrix (works with row-vector: vec * view)
		m_values[0][0] = rx;   m_values[0][1] = ry;   m_values[0][2] = rz;   m_values[0][3] = -(rx * eye.x + ry * eye.y + rz * eye.z);
		m_values[1][0] = ux;   m_values[1][1] = uy;   m_values[1][2] = uz;   m_values[1][3] = -(ux * eye.x + uy * eye.y + uz * eye.z);
		m_values[2][0] = -fx;  m_values[2][1] = -fy;  m_values[2][2] = -fz;  m_values[2][3] =  (fx * eye.x + fy * eye.y + fz * eye.z);
		m_values[3][0] = 0.0f; m_values[3][1] = 0.0f; m_values[3][2] = 0.0f; m_values[3][3] = 1.0f;
	}

	~Mat4()
	{

	}

private:
	float m_values[4][4] = {};
};
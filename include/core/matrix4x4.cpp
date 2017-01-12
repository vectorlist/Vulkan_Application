#include "matrix4x4.h"
#include <memory>
#include <vec3f.h>

const Matrix4x4 Matrix4x4::zero = Matrix4x4(0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f);

Matrix4x4::Matrix4x4()
{
	setToIdentity();
}

Matrix4x4::Matrix4x4(const Matrix4x4 &other)
{
	std::memcpy(m, other.m, sizeof(m));
}

Matrix4x4::Matrix4x4(float other[16])
{
	std::memcpy(m, other, sizeof(m));
}

Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;

	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;

	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;

	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &other) const
{
	Matrix4x4 M;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			M.m[i][j] =
				m[i][0] * other.m[0][j] +
				m[i][1] * other.m[1][j] +
				m[i][2] * other.m[2][j] +
				m[i][3] * other.m[3][j];
		}
	}
	return M;
}

Matrix4x4& Matrix4x4::operator*=(const Matrix4x4 &other)
{
	*this = operator*(other);
	return *this;
}

Matrix4x4 Matrix4x4::operator*(float f) const
{
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result[i][j] = f * m[i][j];
		}
	}
	return result;
}

Matrix4x4& Matrix4x4::operator*=(float f)
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			m[i][j] *= f;
		}
	}
	return *this;
}

void Matrix4x4::setToIdentity()
{
	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;

}

Matrix4x4& Matrix4x4::transpose()
{
	for (int row = 0; row < 4; ++row)
		for (int col = row + 1; col < 4; ++col)
		{
			float tmp = m[row][col];
			m[row][col] = m[col][row];
			m[col][row] = tmp;
		}

	return *this;
}

Matrix4x4 Matrix4x4::transposed() const
{
	Matrix4x4 M;
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			M.m[row][col] = m[col][row];
	return M;
}

Matrix4x4 Matrix4x4::rotatedX(float angle) const
{
	float t = radians(angle);
	float sinT = sin(t);
	float cosT = cos(t);

	Matrix4x4 M = Matrix4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosT, -sinT, 0.0f,
		0.0f, sinT, cosT, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return M;
}

Matrix4x4 Matrix4x4::rotatedY(float angle) const
{
	float t = radians(angle);
	float sinT = sin(t);
	float cosT = cos(t);

	Matrix4x4 M = Matrix4x4(
		cosT, 0.0f, sinT, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinT, 0.0f, cosT, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return M;
}

Matrix4x4 Matrix4x4::rotatedZ(float angle) const
{
	float t = radians(angle);
	float sinT = sin(t);
	float cosT = cos(t);

	Matrix4x4 M = Matrix4x4(
		cosT, -sinT, 0.0f, 0.0f,
		sinT, cosT, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	return M;
}

void Matrix4x4::translate(vec3f t)
{
	Matrix4x4 M;
	M.m[0][3] = t.x;
	M.m[1][3] = t.y;
	M.m[2][3] = t.z;


	*this = *this * M;
}

void Matrix4x4::rotate(AXIS axis, float angle)
{
	//it only working with one axis ex- vec(0,1,0),vec(1,0,0)
	//if u use quanternion method u rotate at once vec(.4,.5,.8) whatever
	//ill implement later quaternion
	float radians = M_PI / 180.f;

	Matrix4x4 rotM;
	switch (axis)
	{
	case AXIS::X:
		rotM.m[0][0] = 1.f;
		rotM.m[1][1] = std::cos(angle * radians);
		rotM.m[1][2] = -sin(angle * radians);
		rotM.m[2][1] = sin(angle * radians);
		rotM.m[2][2] = cos(angle * radians);
		rotM.m[3][3] = 1.f;
		break;
	case AXIS::Y:
		rotM.m[0][0] = cos(angle * radians);
		rotM.m[0][2] = sin(angle * radians);
		rotM.m[1][1] = 1.f;
		rotM.m[2][0] = -sin(angle * radians);
		rotM.m[2][2] = cos(angle * radians);
		rotM.m[3][3] = 1.f;
		break;
	case AXIS::Z:
		rotM.m[0][0] = cos(angle * radians);
		rotM.m[0][1] = -sin(angle * radians);
		rotM.m[1][0] = sin(angle * radians);
		rotM.m[1][1] = cos(angle * radians);
		rotM.m[2][2] = 1.f;
		break;
	}

	*this = *this * rotM;
}

void Matrix4x4::scale(vec3f s)
{
	Matrix4x4 M;
	M.m[0][0] *= s.x;
	M.m[1][1] *= s.y;
	M.m[2][2] *= s.z;

	*this = *this * M;
}

Matrix4x4 Matrix4x4::inverted() const
{
	Matrix4x4 inv;
	float m00 = m[0][0];
	float m01 = m[0][1];
	float m02 = m[0][2];
	float m03 = m[0][3];
	float m10 = m[1][0];
	float m11 = m[1][1];
	float m12 = m[1][2];
	float m13 = m[1][3];
	float m20 = m[2][0];
	float m21 = m[2][1];
	float m22 = m[2][2];
	float m23 = m[2][3];
	float m30 = m[3][0];
	float m31 = m[3][1];
	float m32 = m[3][2];
	float m33 = m[3][3];

	float R23C23 = m22 * m33 - m23 * m32;
	float R23C13 = m21 * m33 - m23 * m31;
	float R23C12 = m21 * m32 - m22 * m31;
	float R23C03 = m20 * m33 - m23 * m30;
	float R23C02 = m20 * m32 - m22 * m30;
	float R23C01 = m20 * m31 - m21 * m30;

	inv[0][0] = +(m11 * R23C23 - m12 * R23C13 + m13 * R23C12);
	inv[1][0] = -(m10 * R23C23 - m12 * R23C03 + m13 * R23C02);
	inv[2][0] = +(m10 * R23C13 - m11 * R23C03 + m13 * R23C01);
	inv[3][0] = -(m10 * R23C12 - m11 * R23C02 + m12 * R23C01);

	float det = m00 * inv[0][0] + m01 * inv[1][0] +
		m02 * inv[2][0] + m03 * inv[3][0];
	if (fabs(det) < MATRIX_EPSILON)
		return false;
	float invDet = 1.0f / det;

	inv[0][1] = -(m01 * R23C23 - m02 * R23C13 + m03 * R23C12);
	inv[1][1] = +(m00 * R23C23 - m02 * R23C03 + m03 * R23C02);
	inv[2][1] = -(m00 * R23C13 - m01 * R23C03 + m03 * R23C01);
	inv[3][1] = +(m00 * R23C12 - m01 * R23C02 + m02 * R23C01);

	float R13C23 = m12 * m33 - m13 * m32;
	float R13C13 = m11 * m33 - m13 * m31;
	float R13C12 = m11 * m32 - m12 * m31;
	float R13C03 = m10 * m33 - m13 * m30;
	float R13C02 = m10 * m32 - m12 * m30;
	float R13C01 = m10 * m31 - m11 * m30;

	inv[0][2] = +(m01 * R13C23 - m02 * R13C13 + m03 * R13C12);
	inv[1][2] = -(m00 * R13C23 - m02 * R13C03 + m03 * R13C02);
	inv[2][2] = +(m00 * R13C13 - m01 * R13C03 + m03 * R13C01);
	inv[3][2] = -(m00 * R13C12 - m01 * R13C02 + m02 * R13C01);

	float R12C23 = m12 * m23 - m13 * m22;
	float R12C13 = m11 * m23 - m13 * m21;
	float R12C12 = m11 * m22 - m12 * m21;
	float R12C03 = m10 * m23 - m13 * m20;
	float R12C02 = m10 * m22 - m12 * m20;
	float R12C01 = m10 * m21 - m11 * m20;

	inv[0][3] = -(m01 * R12C23 - m02 * R12C13 + m03 * R12C12);
	inv[1][3] = +(m00 * R12C23 - m02 * R12C03 + m03 * R12C02);
	inv[2][3] = -(m00 * R12C13 - m01 * R12C03 + m03 * R12C01);
	inv[3][3] = +(m00 * R12C12 - m01 * R12C02 + m02 * R12C01);

	inv *= invDet;
	return inv;
}

vec3f operator*(const Matrix4x4 &m, const vec3f &v)
{
	float x, y, z;
	x = v.x * m.m[0][0] + v.y * m.m[0][1] + v.z * m.m[0][2] + m.m[0][3];
	y = v.x * m.m[1][0] + v.y * m.m[1][1] + v.z * m.m[1][2] + m.m[1][3];
	z = v.x * m.m[2][0] + v.y * m.m[2][1] + v.z * m.m[2][2] + m.m[2][3];

	return vec3f(x, y, z);
}

vec3f Matrix4x4::normal(const vec3f &n)
{
	float x, y, z, w;
	x = n.x * m[0][0] + n.y * m[0][1] + n.z * m[0][2];
	y = n.x * m[1][0] + n.y * m[1][1] + n.z * m[1][2];
	z = n.x * m[2][0] + n.y * m[2][1] + n.z * m[2][2];
	/*w = n.x * m[3][0] + n.y * m[3][1] + n.z * m[3][2] + m[3][3];

	if (w == 1.0f)
	return vec3f(x, y, z);
	else
	return vec3f(x /w, y/w, z/w);*/
	return vec3f(x, y, z).normalized();
}


Matrix4x4 Matrix4x4::perspectiveGL(float fovY, float aspect, float near, float far)
{
	float yscale = 1.f / tan(fovY * 0.5f * M_PI / 180);
	float xscale = yscale / aspect;

	Matrix4x4 M = Matrix4x4(
		xscale, 0.f, 0.f, 0.f,
		0.f, yscale, 0.f, 0.f,
		0.f, 0.f, (far + near) / (near - far), (2 * far * near) / (near - far),
		0.f, 0.f, -1.f, 0.f);
	return M;
}

Matrix4x4 Matrix4x4::perspective(float fovY, float aspect, float near, float far)
{
	//right hand 3D rule

	float yscale = 1.f / tan(fovY / 2.f);
	float xscale = yscale / aspect;

	Matrix4x4 M = Matrix4x4::zero;
	M[0][0] = xscale;
	M[1][1] = yscale;
	M[2][2] = far / (near - far);
	M[2][3] = near * far / (near - far);
	M[3][2] = -1.0f;

	return M;
}

//test
Matrix4x4 Matrix4x4::orthoGL(float width, float height, float near, float far)
{
	//GL right hand rule
	//width = right - left
	//height = top - buttom

	Matrix4x4 M = Matrix4x4::zero;
	M[0][0] = 2.f / width;
	M[1][1] = 2.f / height;
	M[2][2] = 2.f / (near - far);
	M[2][3] = (far + near) / (near - far);
	M[3][3] = 1.f;
	return M;

}

Matrix4x4 Matrix4x4::ortho(float width, float height, float near, float far)
{
	/*right hand*/
	Matrix4x4 M = Matrix4x4::zero;
	M[0][0] = 2.f / width;
	M[1][1] = 2.f / height;
	M[2][2] = 1.f / (near - far);
	M[2][3] = near / (near - far);
	M[3][3] = 1.f;
	return M;
	//left hand
	/*Matrix4x4 M = Matrix4x4::zero;
	M[0][0] = 2.f / width;
	M[1][1] = 2.f / height;
	M[2][2] = 1.f / (far - near);
	M[2][3] = near / (far - near);
	M[3][3] = 1.f;
	return M;*/

}

Matrix4x4 Matrix4x4::lookAtGL(const vec3f &eye, const vec3f &at, const vec3f &upv)
{
	//http://www.3dgep.com/understanding-the-view-matrix/
	vec3f forward = (eye - at).normalized();
	vec3f right = vec3f::cross(upv, forward).normalized();
	vec3f up = vec3f::cross(forward, right).normalized();

	/*viewDir = viewPos−lookAtPos,
	right = Normalize(up,viewDir)
	up= Normalize(viewDir×right)*/

	//translate to view matrix
	//negative is for how far from center to view position
	//equation is View = R + -T
	float x = -vec3f::dot(eye, right);		//eye is viewpos
	float y = -vec3f::dot(eye, up);
	float z = -vec3f::dot(eye, forward);

	//row colum major
	//we must transpose when send to opengl
	Matrix4x4 view = {
		right.x,   right.y,   right.z,    x,
		up.x,      up.y,      up.z,    y,
		forward.x, forward.y, forward.z,    z,
		0.f,       0.f,       0.f,  1.0f
	};

	return view;
}

Matrix4x4 Matrix4x4::perspectiveVK(float fovY, float aspect, float near, float far)
{
	float yscale = 1.f / tan(fovY * 0.5f * M_PI / 180);
	float xscale = yscale / aspect;

	Matrix4x4 M = Matrix4x4(
		xscale, 0.f, 0.f, 0.f,
		0.f, -yscale, 0.f, 0.f,
		0.f, 0.f, (far + near) / (near - far), -1.f,
		0.f, 0.f, (2 * far * near) / (near - far), 0.f);
	return M;
	/*Matrix4x4 Clip = Matrix4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);
	return Clip * M;*/
}


Matrix4x4 Matrix4x4::lookAtVK(const vec3f & eye, const vec3f & at, const vec3f & upv)
{
	//http://www.3dgep.com/understanding-the-view-matrix/
	vec3f forward = (eye - at).normalized();
	vec3f right = vec3f::cross(upv, forward).normalized();
	vec3f up = vec3f::cross(forward, right).normalized();

	float x = -vec3f::dot(eye, right);	
	float y = -vec3f::dot(eye, up);
	float z = -vec3f::dot(eye, forward);
	Matrix4x4 view = {
		right.x,	up.x,		forward.x,		0.0f,
		right.y,    up.y,		forward.y,		0.0f,
		right.z,	up.z,		forward.z,		0.0f,
		x,			y,			z,				1.0f
	};

	return view;
}



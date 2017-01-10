
#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include <assert.h>
#include <vec3f.h>

const float MATRIX_INVERSE_EPSILON = 1e-14f;
const float MATRIX_EPSILON = 1e-5f;

enum class AXIS : unsigned int{
	X = 0, Y, Z
};

struct MatrixType
{
	MatrixType() : identiy(false), translate(false), scale(false), rotation(false){}
	bool hasTransform() const{
		return (rotation || scale || translate);
	}
	bool identiy;
	bool translate;
	bool scale;
	bool rotation;
};

class Matrix4x4
{
public:
	Matrix4x4();
	Matrix4x4(const Matrix4x4 &other);
	Matrix4x4(float other[16]);
	Matrix4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);
	~Matrix4x4(){};

	static const Matrix4x4 zero;

	/*OPERATOR*/

	Matrix4x4 operator*(const Matrix4x4 &other) const;
	Matrix4x4& operator*=(const Matrix4x4 &other);
	Matrix4x4 operator*(float f) const;
	Matrix4x4& operator*=(float f);

	const float* operator[](int i) const;
	float* operator[](int i);

	/*FOR GENERIC NORMAL*/
	vec3f normal(const vec3f &n);

	/*FUNCTION*/
	void setToIdentity();
	Matrix4x4& transpose();
	Matrix4x4 transposed() const;

	/*TRANSFORM*/
	void translate(vec3f t);
	void rotate(AXIS axis, float angle);
	void scale(vec3f s);


	float m[4][4];
	MatrixType type()const{
		return m_type;
	}
	inline MatrixType& type() { return m_type; }

	Matrix4x4 rotatedX(float angle) const;
	Matrix4x4 rotatedY(float angle) const;
	Matrix4x4 rotatedZ(float angle) const;

	Matrix4x4 inverted() const;
protected:
	/*TYPE*/
	MatrixType m_type;

private:

public:
	static Matrix4x4 perspectiveGL(float fovY, float aspect, float near, float far);
	static Matrix4x4 perspectiveQT(float fovY, float aspect, float near, float far);
	static Matrix4x4 perspective(float fovY, float aspect, float near, float far);
	static Matrix4x4 orthoGL(float width, float height, float near, float far);
	static Matrix4x4 ortho(float width, float height, float near, float far);
	static Matrix4x4 lookAt(const vec3f &eye, const vec3f &at, const vec3f &up);


};

vec3f operator*(const Matrix4x4 &m, const vec3f &v);

inline const float* Matrix4x4::operator[](int i) const {
	assert(i >= 0 && i < 4);
	return m[i];
}

inline float* Matrix4x4::operator[](int i) {
	assert(i >= 0 && i < 4);
	return m[i];
}

const inline float radians(float deg)
{
	return M_PI / 180.f * deg;
}

const inline float degree(float rad)
{
	return 180.f / M_PI * rad;
}

#endif // MATRIX4X4_H

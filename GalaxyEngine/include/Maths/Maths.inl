#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <algorithm>

#include "Maths.h"

namespace GALAXY::Math {
#pragma region Math Functions
	inline bool Approximately(float a, float b, float diff) {
		float absoluteDiff = std::abs(a - b);
		return absoluteDiff <= diff;
	}
#pragma endregion

#pragma region Vec2

	template<typename T>
	inline Vec2<T>::Vec2(const std::string& str)
	{
		std::istringstream ss(str);

		char discard;
		ss >> this->x >> discard >> this->y;
	}

	template<typename T>
	template<typename U>
	inline Vec2<T>::Vec2(const Vec2<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
	}

	template<typename T>
	template<typename U>
	Vec2<T>::Vec2(const Vec3<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
	}

	template<typename T>
	template<typename U>
	inline Vec2<T> Vec2<T>::operator=(const Vec2<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
		return *this;
	}

	template<typename T>
	Vec2<T> Vec2<T>::operator+(const Vec2& a) const
	{
		return { x + a.x, y + a.y };
	}

	template<typename T>
	void Vec2<T>::operator+=(const Vec2& a)
	{
		*this = operator+(a);
	}

	template<typename T>
	Vec2<T> Vec2<T>::operator-(const Vec2& a) const
	{
		return { x - a.x, y - a.y };
	}

	template<typename T>
	inline void Vec2<T>::operator-=(const Vec2& a)
	{
		*this = operator-(a);
	}

	template<typename T>
	Vec2<T> Vec2<T>::operator-(void) const
	{
		return { -x, -y };
	}

	template<typename T>
	template<typename U>
	Vec2<T> Vec2<T>::operator*(const Vec2<U>& a) const
	{
		return { x * a.x, y * a.y };
	}

	template<typename T>
	inline void Vec2<T>::operator*=(const Vec2& a)
	{
		*this = operator*(a);
	}

	template<typename T>
	template<typename U>
	Vec2<T> Vec2<T>::operator*(const U& a) const
	{
		return { x * a, y * a };
	}

	template<typename T>
	template<typename U>
	inline void Vec2<T>::operator*=(const U& a)
	{
		*this = operator*(a);
	}

	template<typename T>
	template<typename U>
	Vec2<T> Vec2<T>::operator/(const U& a) const
	{
		return { x / a, y / a };
	}

	template<typename T>
	template<typename U>
	Vec2<T> Vec2<T>::operator/=(const U& a)
	{
		*this = operator/(a);
	}

	template<typename T>
	bool Vec2<T>::operator==(const Vec2& a) const
	{
		return x == a.x && y == a.y;
	}

	template<typename T>
	bool Vec2<T>::operator!=(const Vec2& a) const
	{
		return x != a.x || y != a.y;
	}

	template<typename T>
	T& Vec2<T>::operator[](const size_t a)
	{
		return *((&x) + a);
	}

	template<typename T>
	T Vec2<T>::LengthSquared() const
	{
		return x * x + y * y;
	}

	template<typename T>
	T Vec2<T>::Length() const
	{
		return static_cast<T>(std::sqrt(LengthSquared()));
	}

	template<typename T>
	inline T Vec2<T>::Dot(const Vec2& a) const
	{
		return a.x * x + a.y * y;
	}

	template<typename T>
	inline T Vec2<T>::Cross(const Vec2& a) const
	{
		return x * a.y - y * a.x;
	}

	template<typename T>
	inline Vec2<T> Vec2<T>::Ortho(const Vec2& a) const
	{
		return { -y, x };
	}

	template<typename T>
	void Math::Vec2<T>::Normalize()
	{
		*this = GetNormalize();
	}

	template<typename T>
	Vec2<T> Math::Vec2<T>::GetNormalize() const
	{
		T len = Length();
		if (len != 0)
			return { x / len, y / len };
		return {};
	}

	template<typename T>
	void Vec2<T>::Print(int precision /*= 6*/) const
	{
		std::cout << std::fixed << std::setprecision(precision);
		std::cout << x << ", " << y << std::endl;
	}

	template<typename T>
	std::string Vec2<T>::ToString(int precision /*= 6*/) const
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision);

		oss << x << ", " << y;

		return oss.str();
	}

	template<typename T>
	Vec2<float> Vec2<T>::ToFloat() const
	{
		return Vec2f{ static_cast<float>(x), static_cast<float>(y) };
	}

	template<typename T>
	Vec2<int> Vec2<T>::ToInt() const
	{
		return Vec2i{ static_cast<int>(x), static_cast<int>(y) };
	}

	template<typename T>
	T* Vec2<T>::Data() const
	{
		return const_cast<T*>(reinterpret_cast<const T*>(this));
	}

#pragma endregion

#pragma region Vec3

	template<typename T>
	inline Vec3<T>::Vec3(const std::string& str)
	{
		std::istringstream ss(str);

		char discard;
		ss >> this->x >> discard >> this->y >> discard >> this->z;
	}

	template<typename T>
	template<typename U>
	Vec3<T>::Vec3(const Vec2<U>& xy, T _z)
	{
		x = static_cast<T>(xy.x);
		y = static_cast<T>(xy.y);
		z = _z;
	}

	template<typename T>
	template<typename U>
	Vec3<T>::Vec3(const Vec3<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
		z = static_cast<T>(a.z);
	}

	template<typename T>
	template<typename U>
	Vec3<T>::Vec3(const Vec4<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
		z = static_cast<T>(a.z);
	}

	template<typename T>
	template<typename U>
	Vec3<T> Vec3<T>::operator*(const U& b) const
	{
		return { x * b, y * b, z * b };
	}

	template<typename T>
	template<typename U>
	inline Vec3<T> Vec3<T>::operator/(const U& b) const
	{
		return { x / b, y / b, z / b };
	}

	template<typename T>
	template<typename U>
	inline void Vec3<T>::operator*=(const U& b)
	{
		*this = operator*(b);
	}

	template<typename T>
	template<typename U>
	inline void Vec3<T>::operator/=(const U& b)
	{
		*this = operator/(b);
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator+(const Vec3& b) const
	{
		return { x + b.x, y + b.y, z + b.z };
	}

	template<typename T>
	void Vec3<T>::operator+=(const Vec3& b)
	{
		*this = operator+(b);
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator-(const Vec3& b) const
	{
		return { x - b.x, y - b.y, z - b.z };
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator-(void) const
	{
		return { -x, -y, -z };
	}

	template<typename T>
	void Vec3<T>::operator-=(const Vec3& b)
	{
		*this = operator-(b);
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator*(const Vec3& b) const
	{
		return { x * b.x, y * b.y, z * b.z };
	}

	template<typename T>
	void Vec3<T>::operator*=(const Vec3& b)
	{
		*this = operator*(b);
	}

	template<typename T>
	bool Vec3<T>::operator==(const Vec3& a) const
	{
		return x == a.x && y == a.y && z == a.z;
	}

	template<typename T>
	bool Vec3<T>::operator!=(const Vec3& a) const
	{
		return x != a.x || y != a.y || z != a.z;
	}

	template<typename T>
	T& Vec3<T>::operator[](const size_t a)
	{
		return *((&x) + a);
	}

	template<typename T>
	const T& Vec3<T>::operator[](const size_t a) const
	{
		return *((&x) + a);
	}

	template<typename T>
	T Vec3<T>::LengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	template<typename T>
	T Vec3<T>::Length() const
	{
		return std::sqrt(LengthSquared());
	}

	template<typename T>
	T Vec3<T>::Dot(const Vec3& a) const
	{
		return x * a.x + y * a.y + z * a.z;
	}

	template<typename T>
	Vec3<T> Vec3<T>::Cross(const Vec3& a) const
	{
		return { (y * a.z) - (z * a.y), (z * a.x) - (x * a.z), (x * a.y) - (y * a.x) };
	}

	template<typename T>
	Vec3<T> Vec3<T>::GetNormalize() const
	{
		T len = Length();
		if (len != 0)
			return { x / len, y / len, z / len };
		return {};
	}

	template<typename T>
	void Vec3<T>::Normalize()
	{
		*this = GetNormalize();
	}

	template<typename T>
	T Vec3<T>::Distance(const Vec3& a) const
	{
		T i = a.x - x;
		T j = a.y - y;
		T h = a.z - z;
		return std::sqrt(i * i + j * j + h * h);
	}

	template<typename T>
	Vec3<T> Vec3<T>::Lerp(const Vec3& b, float t) const
	{
		if (t < 0)
			return *this;
		else if (t >= 1)
			return b;
		return (*this) * (1 - t) + b * t;
	}

	template<typename T>
	void Vec3<T>::Print(int precision /*= 6*/) const
	{
		std::cout << std::fixed << std::setprecision(precision);
		std::cout << x << ", " << y << ", " << z << std::endl;
	}

	template<typename T>
	std::string Vec3<T>::ToString(int precision /*= 6*/) const
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision);
		oss << x << ", " << y << ", " << z;
		return oss.str();
	}

	template<typename T>
	Quat Vec3<T>::ToQuaternion() const
	{
		float halfToRad = 0.5f * DegToRad;  // Convert degrees to radians

		float xRad = x * halfToRad;
		float yRad = y * halfToRad;
		float zRad = z * halfToRad;

		float sinX = std::sin(xRad);
		float cosX = std::cos(xRad);
		float sinY = std::sin(yRad);
		float cosY = std::cos(yRad);
		float sinZ = std::sin(zRad);
		float cosZ = std::cos(zRad);

		Quat result;

		result.w = cosY * cosX * cosZ + sinY * sinX * sinZ;
		result.x = cosY * sinX * cosZ + sinY * cosX * sinZ;
		result.y = sinY * cosX * cosZ - cosY * sinX * sinZ;
		result.z = cosY * cosX * sinZ - sinY * sinX * cosZ;

		return result;
	}

	template<typename T>
	T* Vec3<T>::Data() const
	{
		return const_cast<T*>(reinterpret_cast<const T*>(this));
	}

#pragma endregion

#pragma region Vec4
	template<typename T>
	inline Vec4<T>::Vec4(const std::string& str)
	{
		std::istringstream ss(str);

		char discard;
		ss >> this->x >> discard >> this->y >> discard >> this->z >> discard >> this->w;
	}

	template<typename T>
	template<typename U>
	Vec4<T>::Vec4(const Vec2<U>& xy, T _z /*= 0*/, T _w /*= 0*/)
	{
		x = static_cast<T>(xy.x);
		y = static_cast<T>(xy.y);
		z = _z;
		w = _w;
	}

	template<typename T>
	template<typename U>
	Vec4<T>::Vec4(const Vec3<U>& xyz, T _w /*= 0*/)
	{
		x = static_cast<T>(xyz.x);
		y = static_cast<T>(xyz.y);
		z = static_cast<T>(xyz.z);
		w = _w;
	}

	template<typename T>
	template<typename U>
	Vec4<T>::Vec4(const Vec4<U>& a)
	{
		x = static_cast<T>(a.x);
		y = static_cast<T>(a.y);
		z = static_cast<T>(a.z);
		w = static_cast<T>(a.w);
	}

	template<typename T>
	inline Vec4<T> Vec4<T>::operator+(const Vec4& b) const {
		return { x + b.x, y + b.y, z + b.z, w + b.w };
	}

	template<typename T>
	inline Vec4<T> Vec4<T>::operator-(const Vec4& b) const {
		return { x - b.x, y - b.y, z - b.z, w - b.w };
	}

	template<typename T>
	inline Vec4<T> Vec4<T>::operator-(void) const {
		return { -x, -y, -z, -w };
	}

	template<typename T>
	inline Vec4<T> Vec4<T>::operator*(const Vec4& b) const {
		return Vec4(x * b.x, y * b.y, z * b.z, w * b.w);
	}

	template<typename T>
	template<typename U>
	inline Vec4<T> Vec4<T>::operator*(const U& b) const {
		return { x * b, y * b, z * b, w * b };
	}

	template<typename T>
	template<typename U>
	inline Vec4<T> Vec4<T>::operator/(const U& b) const {
		return { x / b, y / b, z / b, w / b };
	}

	template<typename T>
	inline void Vec4<T>::operator+=(const Vec4& b) {
		*this = operator+(b);
	}

	template<typename T>
	inline void Vec4<T>::operator-=(const Vec4& b) {
		*this = operator-(b);
	}

	template<typename T>
	inline void Vec4<T>::operator*=(const Vec4& b) {
		*this = operator*(b);
	}

	template<typename T>
	template<typename U>
	inline void Vec4<T>::operator*=(const U& b) {
		*this = operator*(b);
	}

	template<typename T>
	template<typename U>
	inline void Vec4<T>::operator/=(const U& b) {
		*this = operator/(b);
	}

	template<typename T>
	inline bool Vec4<T>::operator==(const Vec4& b) const {
		return (x == b.x && y == b.y && z == b.z && w == b.w);
	}

	template<typename T>
	inline bool Vec4<T>::operator!=(const Vec4& b) const {
		return (x != b.x || y != b.y || z != b.z || w != b.w);
	}

	template<typename T>
	inline T& Vec4<T>::operator[](const size_t a) {
		return *((&x) + a);
	}

	template<typename T>
	inline const T& Vec4<T>::operator[](const size_t a) const {
		return *((&x) + a);
	}

	template<typename T>
	inline T Vec4<T>::LengthSquared() const {
		return (x * x + y * y + z * z + w * w);
	}

	template<typename T>
	inline T Vec4<T>::Length() const {
		return std::sqrt(LengthSquared());
	}

	template<typename T>
	inline T Vec4<T>::Dot(const Vec4& a) const {
		return (x * a.x + y * a.y + z * a.z + w * a.w);
	}

	template<typename T>
	inline T Vec4<T>::Distance(const Vec4& a) const {
		return (a - *this).Length();
	}

	template<typename T>
	Vec4<T> Vec4<T>::GetHomogenize() const
	{
		return { ToVector3() / w };
	}

	template<typename T>
	void Vec4<T>::Homogenize()
	{
		*this = GetHomogenize();
	}

	template<typename T>
	inline void Vec4<T>::Normalize()
	{
		*this = GetNormalize();
	}

	template<typename T>
	inline Vec4<T> Vec4<T>::GetNormalize() const {
		T len = Length();
		if (len != 0)
			return operator/(len);
		return {};
	}

	template<typename T>
	void Vec4<T>::Print(int precision /*= 6*/) const
	{
		std::cout << std::fixed << std::setprecision(precision);
		std::cout << x << ", " << y << ", " << z << ", " << w << std::endl;
	}

	template<typename T>
	Vec3<T> Vec4<T>::ToVector3() const
	{
		return { x, y, z };
	}

	template<typename T>
	std::string Vec4<T>::ToString(int precision /*= 6*/) const
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision);
		oss << x << ", " << y << ", " << z << ", " << w;
		return oss.str();
	}

	template<typename T>
	T* Vec4<T>::Data() const
	{
		return const_cast<T*>(reinterpret_cast<const T*>(this));
	}

#pragma endregion

#pragma  region Mat4
	inline Mat4::Mat4(float diagonal)
	{
		content[0][0] = 1;
		content[1][1] = 1;
		content[2][2] = 1;
		content[3][3] = 1;
	}

	inline Mat4::Mat4(const float* data)
	{
		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				content[i][j] = data[i * 4 + j];
			}
		}
	}

	inline Mat4::Mat4(const double* data)
	{
		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				content[i][j] = static_cast<float>(data[i * 4 + j]);
			}
		}
	}

	inline Mat4 Mat4::operator*(const Mat4& a) const
	{
		Mat4 out;
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t i = 0; i < 4; i++)
			{
				for (size_t k = 0; k < 4; k++)
					out.content[j][i] += content[j][k] * a.content[k][i];
			}
		}
		return out;
	}

	template<typename U>
	inline Vec4<U> Mat4::operator*(const Vec4<U>& a) const
	{
		Vec4<U> out;
		for (size_t i = 0; i < 4; i++)
		{
			float res = 0;
			for (size_t k = 0; k < 4; k++) res += content[i][k] * a[k];
			out[i] = res;
		}
		return out;
	}

	template<typename U>
	inline Vec3<U> Mat4::operator*(const Vec3<U>& point) const
	{
		Vec3<U> res;
		float w;
		res.x = content[0][0] * point.x + content[1][0] * point.y + content[2][0] * point.z + content[3][0];
		res.y = content[0][1] * point.x + content[1][1] * point.y + content[2][1] * point.z + content[3][1];
		res.z = content[0][2] * point.x + content[1][2] * point.y + content[2][2] * point.z + content[3][2];
		w = content[0][3] * point.x + content[1][3] * point.y + content[2][3] * point.z + content[3][3];

		w = 1.f / w;
		res.x *= w;
		res.y *= w;
		res.z *= w;
		return res;
	}

	inline Mat4 Mat4::operator+(const Mat4& a) const
	{
		Mat4 tmp;
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t i = 0; i < 4; i++)
			{
				tmp.content[j][i] = content[j][i] + a.content[j][i];
			}
		}
		return tmp;
	}

	inline float* Mat4::operator[](const size_t a)
	{
		return content[a];
	}

	inline Mat4 Mat4::CreateProjectionMatrix(float _fov, float _aspect, float _near, float _far)
	{
		float tanHalfFov = std::tan(_fov * DegToRad * 0.5f);

		Mat4 projectionMatrix = Mat4();
		projectionMatrix[0][0] = 1.0f / (_aspect * tanHalfFov);
		projectionMatrix[1][1] = 1.0f / tanHalfFov;
		projectionMatrix[2][2] = (_far + _near) / (_far - _near);
		projectionMatrix[3][2] = 1.0f;
		projectionMatrix[2][3] = -(2.0f * _far * _near) / (_far - _near);
		projectionMatrix[3][3] = 0.0f;

		return projectionMatrix;
	}

	template<typename U>
	inline Mat4 Mat4::CreateTranslationMatrix(const Vec3<U>& translation)
	{
		Mat4 out(1);
		for (size_t i = 0; i < 3; i++)
			out[i][3] = translation[i];
		return out;
	}

	template<typename U>
	inline Mat4 Mat4::CreateScaleMatrix(const Vec3<U>& scale)
	{
		Mat4 out(1);
		for (size_t i = 0; i < 3; i++)
			out[i][i] = scale[i];
		return out;
	}

	template<typename U>
	inline Mat4 Mat4::CreateRotationMatrix(const Vec3<U>& rotation)
	{
		return Quat::FromEuler(rotation).ToRotationMatrix();
	}

	template<typename U>
	inline Mat4 Mat4::CreateTransformMatrix(const Vec3<U>& position, const Vec3<U>& rotation, const Vec3<U>& scale)
	{
		return CreateTranslationMatrix(position) * CreateRotationMatrix(rotation) * CreateScaleMatrix(scale);
	}

	template<typename U>
	inline Mat4 Mat4::CreateTransformMatrix(const Vec3<U>& position, const Quat& rotation, const Vec3<U>& scale)
	{
		return CreateTranslationMatrix(position) * rotation.ToRotationMatrix() * CreateScaleMatrix(scale);
	}

	inline Mat4 Mat4::CreatePerspectiveProjectionMatrix(float Near, float Far, float fov)
	{
		float s = 1.0f / std::tan((fov / 2.0f) * DegToRad);
		float param1 = -Far / (Far - Near);
		float param2 = param1 * Near;
		Mat4 out;
		out[0][0] = s;
		out[1][1] = s;
		out[2][2] = param1;
		out[3][2] = -1;
		out[2][3] = param2;
		return out;
	}
	/*

	inline Quat Mat4::ToQuaternion()
	{
		float w = sqrtf(1 + at(0, 0) + at(1, 1) + at(2, 2)) / 2;
		return Quat((at(2, 1) - at(1, 2)) / (4 * w), (at(0, 2) - at(2, 0)) / (4 * w), (at(1, 0) - at(0, 1)) / (4 * w), w);
	}
	*/

	template<typename U>
	inline Vec3<U> Mat4::GetPosition() const
	{
		return { content[3][0], content[3][1], content[3][2] };
	}

	template<typename U>
	inline Vec3<U> Mat4::GetScale() const
	{
		// World Scale equal length of columns of the model matrix.
		float x = Vec3f(content[0][0], content[0][1], content[0][2]).Length();
		float y = Vec3f(content[1][0], content[1][1], content[1][2]).Length();
		float z = Vec3f(content[2][0], content[2][1], content[2][2]).Length();
		return { x, y, z };
	}

	template<typename U>
	inline Vec3<U> Mat4::GetEulerRotation() const
	{
		float sy = sqrt(content[0][0] * content[0][0] + content[0][1] * content[0][1]);

		bool singular = sy < 1e-6;

		float x, y, z;

		if (!singular)
		{
			x = atan2( content[1][2], content[2][2]);
			y = atan2(-content[0][2], sy);
			z = atan2(content[0][1], content[0][0]);
		}
		else
		{
			x = atan2(-content[2][1], content[1][1]);
			y = atan2(-content[0][2], sy);
			z = 0;
		}

		return -Vec3<U>(x, y, z) * RadToDeg;
	}

	inline Mat4 Mat4::CreateInverseMatrix() const
	{
		// Find determinant of matrix
		Mat4 inverse;
		float det = GetDeterminant(4);
		if (det == 0)
		{
			std::cout << "ERROR with Inverse Matrix" << std::endl;
			return Mat4::Identity();
		}

		// Find adjoint
		Mat4 adj = CreateAdjMatrix();

		// Find Inverse using formula "inverse(A) = adj(A)/det(A)"
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				inverse.content[i][j] = adj.content[i][j] / float(det);

		return inverse;
	}

	inline Mat4 Mat4::CreateAdjMatrix() const
	{
		// temp is used to store cofactors of matrix
		Mat4 temp;
		Mat4 adj;
		int sign = 1;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				// Get cofactor of matrix[i][j]
				temp = GetCofactor(i, j, 4);

				// sign of adj positive if sum of row
				// and column indexes is even.
				sign = ((i + j) % 2 == 0) ? 1 : -1;

				// Interchanging rows and columns to get the
				// transpose of the cofactor matrix
				adj.content[j][i] = (float)((sign) * (temp.GetDeterminant(3)));
			}
		}
		return adj;
	}

	inline Mat4 Mat4::GetCofactor(int p, int q, int n) const
	{
		Mat4 mat;
		int i = 0, j = 0;
		// Looping for each element of the matrix
		for (int row = 0; row < n; row++)
		{
			for (int col = 0; col < n; col++)
			{
				//  Copying into temporary matrix only those element
				//  which are not in given row and column
				if (row != p && col != q)
				{
					mat.content[i][j++] = content[row][col];

					// Row is filled, so increase row index and
					// reset col index
					if (j == n - 1)
					{
						j = 0;
						i++;
					}
				}
			}
		}
		return mat;
	}

	inline float Mat4::GetDeterminant(float n) const
	{
		if (n == 2)
		{
			float result = content[0][0] * content[1][1] - content[1][0] * content[0][1];
			return result;
		}
		else if (n == 3)
		{
			float result = content[0][0] * content[1][1] * content[2][2]
				- content[0][0] * content[2][1] * content[1][2]
				+ content[1][0] * content[2][1] * content[0][2]
				- content[1][0] * content[0][1] * content[2][2]
				+ content[2][0] * content[0][1] * content[1][2]
				- content[2][0] * content[1][1] * content[0][2];
			return result;
		}
		else if (n == 4)
		{
			float result = content[0][0] * (content[1][1] * content[2][2] * content[3][3] // a(fkp
				- content[1][1] * content[3][2] * content[2][3] //flo
				- content[2][1] * content[1][2] * content[3][3] //gjp
				+ content[2][1] * content[3][2] * content[1][3] //gln
				+ content[3][1] * content[1][2] * content[2][3] //hjo
				- content[3][1] * content[2][2] * content[1][3]) // hkn

				- content[1][0] * (content[0][1] * content[2][2] * content[3][3] //b(ekp
					- content[0][1] * content[3][2] * content[2][3] // elo
					- content[2][1] * content[0][2] * content[3][3] //gip
					+ content[2][1] * content[3][2] * content[0][3] //glm
					+ content[3][1] * content[0][2] * content[2][3] //hio
					- content[3][1] * content[2][2] * content[0][3]) //hkm

				+ content[2][0] * (content[0][1] * content[1][2] * content[3][3] // c(ejp
					- content[0][1] * content[3][2] * content[1][3] //eln
					- content[1][1] * content[0][2] * content[3][3] //fip
					+ content[1][1] * content[3][2] * content[0][3] //flm
					+ content[3][1] * content[0][2] * content[1][3] //hin
					- content[3][1] * content[1][2] * content[0][3]) //hjm

				- content[3][0] * (content[0][1] * content[1][2] * content[2][3] // d(ejo
					- content[0][1] * content[2][2] * content[1][3] //ekn
					- content[1][1] * content[0][2] * content[2][3] //fio
					+ content[1][1] * content[2][2] * content[0][3] //fkm
					+ content[2][1] * content[0][2] * content[1][3] //gin
					- content[2][1] * content[1][2] * content[0][3]); //gjm
			return result;
		}
		else return 0.0f;
	}

	inline Mat4 Mat4::GetTranspose() const
	{
		Mat4 transpose = *this;
		float temp;

		for (int i = 0; i < 4; i++) {
			for (int j = i + 1; j < 4; j++) {
				temp = transpose[i][j];
				transpose[i][j] = transpose[j][i];
				transpose[j][i] = temp;
			}
		}
		return transpose;
	}

	template<typename U>
	inline Vec3<U> Mat4::MultiplyPoint3x4(Vec3<U> point)
	{
		Vec3<U> res;
		res.x = content[0][0] * point.x + content[0][1] * point.y + content[0][2] * point.z + content[0][3];
		res.y = content[1][0] * point.x + content[1][1] * point.y + content[1][2] * point.z + content[1][3];
		res.z = content[2][0] * point.x + content[2][1] * point.y + content[2][2] * point.z + content[2][3];
		return res;
	}

	template<typename U>
	inline Vec3<U> Mat4::MultiplyVector(Vec3<U> vector)
	{
		Vec3<U> res;
		res.x = content[0][0] * vector.x + content[0][1] * vector.y + content[0][2] * vector.z;
		res.y = content[1][0] * vector.x + content[1][1] * vector.y + content[1][2] * vector.z;
		res.z = content[2][0] * vector.x + content[2][1] * vector.y + content[2][2] * vector.z;
		return res;
	}

	inline float* Mat4::Data() const
	{
		return const_cast<float*>(reinterpret_cast<const float*>(this));
	}

	inline void Mat4::Print() const
	{
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (content[j][i] >= 0.0f) printf(" ");
				printf("%.6f", content[j][i]);
			}
			printf("\n");
		}
		printf("\n");
	}

	inline std::string Math::Mat4::ToString() const
	{
		std::string print;
		for (int j = 0; j < 4; j++)
		{
			print += "{";
			for (int i = 0; i < 4; i++)
			{
				print += " ";
				print += std::to_string(content[j][i]);
			}
			print += "}";
		}
		return print;
	}

	inline Quat Mat4::GetRotation() const
	{
		// Extracting the rotation from the matrix
		float trace = content[0][0] + content[1][1] + content[2][2];

		if (trace > 0)
		{
			float s = 0.5f / std::sqrt(trace + 1.0f);
			float w = 0.25f / s;
			float x = (content[1][2] - content[2][1]) * s;
			float y = (content[2][0] - content[0][2]) * s;
			float z = (content[0][1] - content[1][0]) * s;
			return Quat(x, y, z, w).GetInverse();
		}
		else if (content[0][0] > content[1][1] && content[0][0] > content[2][2])
		{
			float s = 2.0f * std::sqrt(1.0f + content[0][0] - content[1][1] - content[2][2]);
			float x = 0.25f * s;
			float w = (content[1][2] - content[2][1]) / s;
			float y = (content[1][0] + content[0][1]) / s;
			float z = (content[2][0] + content[0][2]) / s;
			return Quat(x, y, z, w).GetInverse();
		}
		else if (content[1][1] > content[2][2])
		{
			float s = 2.0f * std::sqrt(1.0f + content[1][1] - content[0][0] - content[2][2]);
			float y = 0.25f * s;
			float w = (content[2][0] - content[0][2]) / s;
			float x = (content[1][0] + content[0][1]) / s;
			float z = (content[2][1] + content[1][2]) / s;
			return Quat(x, y, z, w).GetInverse();
		}
		else
		{
			float s = 2.0f * std::sqrt(1.0f + content[2][2] - content[0][0] - content[1][1]);
			float w = (content[0][1] - content[1][0]) / s;
			float x = (content[2][0] + content[0][2]) / s;
			float y = (content[2][1] + content[1][2]) / s;
			float z = 0.25f * s;
			return Quat(x, y, z, w).GetInverse();
		}
	}
#pragma  endregion

#pragma region Quaternion

	inline Quat::Quat(const std::string& str)
	{
		std::istringstream ss(str);

		char discard;
		ss >> this->x >> discard >> this->y >> discard >> this->z >> discard >> this->w;
	}

	inline Quat Quat::operator+(const Quat& a) const
	{
		return Quat(x + a.x, y + a.y, z + a.z, w + a.w);
	}

	inline Quat Quat::operator-(const Quat& a) const
	{
		return Quat(x - a.x, y - a.y, z - a.z, w - a.w);
	}

	inline Quat Quat::operator*(const Quat& a) const
	{
		return Quat(
			w * a.x + x * a.w + y * a.z - z * a.y,
			w * a.y + y * a.w + z * a.x - x * a.z,
			w * a.z + z * a.w + x * a.y - y * a.x,
			w * a.w - x * a.x - y * a.y - z * a.z);
	}

	inline Quat Quat::operator*(const float& a) const
	{
		return Quat(this->x * a, this->y * a, this->z * a, this->w * a);
	}
	template<typename U>
	inline Vec3<U> Quat::operator*(const Vec3<U>& a) const
	{
		Vec3<U> vector;
		float ax = x * 2.f;
		float ay = y * 2.f;
		float az = z * 2.f;
		float xx = x * ax;
		float yy = y * ay;
		float zz = z * az;
		float xy = x * ay;
		float xz = x * az;
		float yz = y * az;
		float wx = w * ax;
		float wy = w * ay;
		float wz = w * az;
		return { (1.f - (yy + zz)) * a.x + (xy - wz) * a.y + (xz + wy) * a.z ,
				(xy + wz) * a.x + (1.f - (xx + zz)) * a.y + (yz - wx) * a.z ,
				(xz - wy) * a.x + (yz + wx) * a.y + (1.f - (xx + yy)) * a.z };
	}

	inline float& Quat::operator[](const size_t index)
	{
		return *((&x) + index);
	}

	template<typename U>
	inline Quat Quat::AngleAxis(float angle, Vec3<U> axis)
	{
		float rad = angle * DegToRad;
		axis.Normalize();
		Quat q;
		q.w = std::cos(rad / 2);
		q.x = std::sin(rad / 2) * axis.x;
		q.y = std::sin(rad / 2) * axis.y;
		q.z = std::sin(rad / 2) * axis.z;
		return q;
	}

	template<typename U>
	Quat Quat::FromEuler(Vec3<U> euler)
	{
		float halfToRad = 0.5f * DegToRad;  // Convert degrees to radians

		float xRad = euler.x * halfToRad;
		float yRad = euler.y * halfToRad;
		float zRad = euler.z * halfToRad;

		float sinX = std::sin(xRad);
		float cosX = std::cos(xRad);
		float sinY = std::sin(yRad);
		float cosY = std::cos(yRad);
		float sinZ = std::sin(zRad);
		float cosZ = std::cos(zRad);

		Quat result;

		result.w = cosY * cosX * cosZ + sinY * sinX * sinZ;
		result.x = cosY * sinX * cosZ + sinY * cosX * sinZ;
		result.y = sinY * cosX * cosZ - cosY * sinX * sinZ;
		result.z = cosY * cosX * sinZ - sinY * sinX * cosZ;

		return result;
	}

	template<typename U>
	Quat Quat::LookRotation(Vec3<U> forward, Vec3<U> up)
	{
		forward.Normalize();
		Vec3<U>  vector = forward.GetNormalize();
		Vec3<U>  vector2 = up.Cross(vector).GetNormalize();
		Vec3<U>  vector3 = vector.Cross(vector2);
		float m00 = vector2.x;
		float m01 = vector2.y;
		float m02 = vector2.z;
		float m10 = vector3.x;
		float m11 = vector3.y;
		float m12 = vector3.z;
		float m20 = vector.x;
		float m21 = vector.y;
		float m22 = vector.z;

		float num8 = (m00 + m11) + m22;
		Quat quaternion;
		if (num8 > 0.f)
		{
			float num = std::sqrt(num8 + 1.f);
			quaternion.w = num * 0.5f;
			num = 0.5f / num;
			quaternion.x = (m12 - m21) * num;
			quaternion.y = (m20 - m02) * num;
			quaternion.z = (m01 - m10) * num;
			return quaternion;
		}
		if ((m00 >= m11) && (m00 >= m22))
		{
			float num7 = std::sqrt(((1.f + m00) - m11) - m22);
			float num4 = 0.5f / num7;
			quaternion.x = 0.5f * num7;
			quaternion.y = (m01 + m10) * num4;
			quaternion.z = (m02 + m20) * num4;
			quaternion.w = (m12 - m21) * num4;
			return quaternion;
		}
		if (m11 > m22)
		{
			float num6 = std::sqrt(((1.f + m11) - m00) - m22);
			float num3 = 0.5f / num6;
			quaternion.x = (m10 + m01) * num3;
			quaternion.y = 0.5f * num6;
			quaternion.z = (m21 + m12) * num3;
			quaternion.w = (m20 - m02) * num3;
			return quaternion;
		}
		float num5 = (float)sqrtf(((1.f + m22) - m00) - m11);
		float num2 = 0.5f / num5;
		return { (m20 + m02) * num2 , (m21 + m12) * num2, 0.5f * num5,(m01 - m10) * num2 };
	}


	inline Quat Quat::SLerp(const Quat& a, const Quat& b, float time)
	{
		if (time < 0.0f)
			return a;
		else if (time >= 1.0f)
			return b;
		float d = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		float s0, s1;
		float sd = (float)((d > 0.0f) - (d < 0.0f));

		d = fabs(d);

		if (d < 0.9995f)
		{
			float s = std::sqrt(1.0f - d * d);
			float a = std::atan2(s, d);
			float c = std::cos(time * a);


			s1 = std::sqrt(1.0f - c * c) / s;
			s0 = c - d * s1;
		}
		else
		{
			s0 = 1.0f - time;
			s1 = time;
		}

		return a * s0 + b * sd * s1;
	}

	inline void Quat::Inverse()
	{
		*this = GetInverse();
	}
	inline Quat Quat::GetInverse() const
	{
		float d = w * w + x * x + y * y + z * z;
		if (x == 0 && y == 0 && z == 0 && w == 0)
			return *this;
		else
			return Quat(-x / d, -y / d, -z / d, w / d);
	}
	inline void Quat::Normalize()
	{
		*this = GetNormal();
	}
	inline Quat Quat::GetNormal() const
	{
		float mag = std::sqrt(Dot(*this));

		if (mag < FLT_MIN)
			return Quat::Identity();
		else
			return Quat(x / mag, y / mag, z / mag, w / mag);
	}

	inline void Quat::Conjugate()
	{
		*this = GetConjugate();
	}

	inline Quat Quat::GetConjugate() const
	{
		return Quat(-x, -y, -z, w);
	}

	inline float Quat::Dot(const Quat& a) const
	{
		return x * a.x + y * a.y + z * a.z + w * a.w;
	}

	static float NormalizeAngle(float angle)
	{
		float modAngle = std::fmod(angle, 360.0f);

		if (modAngle < 0.0f)
			return modAngle + 360.0f;
		else
			return modAngle;
	}

	template<typename U>
	static Vec3<U> NormalizeAngles(Vec3<U>  angles)
	{
		angles.x = NormalizeAngle(angles.x);
		angles.y = NormalizeAngle(angles.y);
		angles.z = NormalizeAngle(angles.z);
		return angles;
	}

	inline Vec3f Quat::ToEuler() const
	{
		return ToEuler<float>();
	}
	template<typename U>
	inline Vec3<U> Quat::ToEuler() const
	{
		float sqw = w * w;
		float sqx = x * x;
		float sqy = y * y;
		float sqz = z * z;
		float unit = sqx + sqy + sqz + sqw;
		float test = x * w - y * z;
		Vec3<U> v;

		if (test > 0.4995f * unit)
		{ // singularity at north pole
			v.y = 2.f * std::atan2(y, x);
			v.x = PI / 2;
			v.z = 0;
			return NormalizeAngles(v * RadToDeg);
		}
		if (test < -0.4995f * unit)
		{ // singularity at south pole
			v.y = -2.f * std::atan2(y, x);
			v.x = -PI / 2;
			v.z = 0;
			return NormalizeAngles(v * RadToDeg);
		}
		Quat q = Quat(w, z, x, y);
		v.y = std::atan2(2.f * q.x * q.w + 2.f * q.y * q.z, 1 - 2.f * (q.z * q.z + q.w * q.w));     // Yaw
		v.x = std::asin(2.f * (q.x * q.z - q.w * q.y));                             // Pitch
		v.z = std::atan2(2.f * q.x * q.y + 2.f * q.z * q.w, 1 - 2.f * (q.y * q.y + q.z * q.z));      // Roll
		return NormalizeAngles(v * RadToDeg);
	}

	inline Mat4 Quat::ToRotationMatrix() const
	{
		// Precalculate coordinate products
		float _x = x * 2.0F;
		float _y = y * 2.0F;
		float _z = z * 2.0F;
		float xx = x * _x;
		float yy = y * _y;
		float zz = z * _z;
		float xy = x * _y;
		float xz = x * _z;
		float yz = y * _z;
		float wx = w * _x;
		float wy = w * _y;
		float wz = w * _z;

		// Calculate 3x3 matrix from orthonormal basis
		Mat4 m;
		m[0][0] = 1.0f - (yy + zz);
		m[1][0] = xy + wz;
		m[2][0] = xz - wy;
		m[3][0] = 0.0F;

		m[0][1] = xy - wz;
		m[1][1] = 1.0f - (xx + zz);
		m[2][1] = yz + wx;
		m[3][1] = 0.0F;

		m[0][2] = xz + wy;
		m[1][2] = yz - wx;
		m[2][2] = 1.0f - (xx + yy);
		m[3][2] = 0.0F;

		m[0][3] = 0.0F;
		m[1][3] = 0.0F;
		m[2][3] = 0.0F;
		m[3][3] = 1.0F;

		return m;
	}

	inline void Quat::Print() const
	{
		printf("Quaternion { %f, %f, %f, %f}\n", x, y, z, w);
	}
	inline std::string Quat::ToString(int precision) const
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision);
		oss << x << ", " << y << ", " << z << ", " << w;
		return oss.str();
	}
#pragma endregion
}
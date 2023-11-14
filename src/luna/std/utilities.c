#include "luna/std/utilities.h"

inline int32_t imin(int32_t x, int32_t y) { return (x < y) ? x : y; }
inline int32_t imax(int32_t x, int32_t y) { return (x > y) ? x : y; }

Vector2 Vector2RotatePoint(Vector2 v1, Vector2 v2, float angle) {
	Vector2 result = { 0 };
	float sinres = sinf(angle);
	float cosres = cosf(angle);

	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	float xr = result.x * cosres - result.y * sinres;
	float yr = result.x * sinres + result.y * cosres;
	result.x = xr + v2.x;
	result.y = yr + v2.y;

	return result;
}

Vector2 Vector2ReflectPoint(Vector2 v1, Vector2 v2) {
	Vector2 result = { 0 };

	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.x = -result.x;
	result.y = -result.y;
	result.x += v2.x;
	result.y += v2.y;

	return result;
}

Vector2i Vector2iZero(void) {
	Vector2i result = { 0, 0 };
	return result;
}

Vector2i Vector2iOne(void) {
	Vector2i result = { 1, 1 };
	return result;
}

Vector2i Vector2iAdd(Vector2i v1, Vector2i v2) {
	Vector2i result = { v1.x + v2.x, v1.y + v2.y };
	return result;
}

Vector2i Vector2iAddValue(Vector2i v, int32_t add) {
	Vector2i result = { v.x + add, v.y + add };
	return result;
}

Vector2i Vector2iSubtract(Vector2i v1, Vector2i v2) {
	Vector2i result = { v1.x - v2.x, v1.y - v2.y };
	return result;
}

Vector2i Vector2iSubtractValue(Vector2i v, int32_t sub) {
	Vector2i result = { v.x - sub, v.y - sub };
	return result;
}

float Vector2iLength(Vector2i v) {
	float result = sqrtf((float)((v.x*v.x) + (v.y*v.y)));
	return result;
}

int32_t Vector2iLengthSqr(Vector2i v) {
	int32_t result = (v.x*v.x) + (v.y*v.y);
	return result;
}

int32_t Vector2iDotProduct(Vector2i v1, Vector2i v2) {
	int32_t result = (v1.x*v2.x + v1.y*v2.y);
	return result;
}

float Vector2iDistance(Vector2i v1, Vector2i v2) {
	float result = sqrtf((float)((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y)));
	return result;
}

int32_t Vector2iDistanceSqr(Vector2i v1, Vector2i v2) {
	int32_t result = ((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
	return result;
}

float Vector2iAngle(Vector2i v1, Vector2i v2) {
	float result = atan2f((float)(v2.y - v1.y), (float)(v2.x - v1.x));
	return result;
}

Vector2i Vector2iMultiply(Vector2i v1, Vector2i v2) {
	Vector2i result = { v1.x*v2.x, v1.y*v2.y };
	return result;
}

Vector2i Vector2iMultiplyValue(Vector2i v, int32_t val) {
	Vector2i result = { v.x*val, v.y*val };
	return result;
}

Vector2i Vector2iNegate(Vector2i v) {
	Vector2i result = { -v.x, -v.y };
	return result;
}

Vector2i Vector2iDivide(Vector2i v1, Vector2i v2) {
	Vector2i result = { v1.x/v2.x, v1.y/v2.y };
	return result;
}

Vector2i Vector2iDivideValue(Vector2i v, int32_t scale) {
	Vector2i result = { v.x/scale, v.y/scale };
	return result;
}

Vector2i Vector2iReflect(Vector2i v, Vector2i normal) {
	Vector2i result = { 0 };
	uint32_t dotProduct = (v.x*normal.x + v.y*normal.y);
	result.x = v.x - (2*normal.x)*dotProduct;
	result.y = v.y - (2*normal.y)*dotProduct;
	return result;
}

Vector2i Vector2iClamp(Vector2i v, Vector2i min, Vector2i max) {
	Vector2i result = { 0 };
	result.x = imin(max.x, imax(min.x, v.x));
	result.y = imin(max.y, imax(min.y, v.y));
	return result;
}

int Vector2iEquals(Vector2i p, Vector2i q) {
	int result = (p.x == q.x) && (p.y == q.y);
	return result;
}

Vector3i Vector3iZero(void) {
	Vector3i result = { 0, 0, 0 };
	return result;
}

Vector3i Vector3iOne(void) {
	Vector3i result = { 1, 1, 1 };
	return result;
}

Vector3i Vector3iAdd(Vector3i v1, Vector3i v2) {
	Vector3i result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return result;
}

Vector3i Vector3iAddValue(Vector3i v, int32_t add) {
	Vector3i result = { v.x + add, v.y + add, v.z + add };
	return result;
}

Vector3i Vector3iSubtract(Vector3i v1, Vector3i v2) {
	Vector3i result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return result;
}

Vector3i Vector3iSubtractValue(Vector3i v, int32_t sub) {
	Vector3i result = { v.x - sub, v.y - sub, v.z - sub };
	return result;
}

Vector3i Vector3iMultiply(Vector3i v1, Vector3i v2) {
	Vector3i result = { v1.x*v2.x, v1.y*v2.y, v1.z*v2.z };
	return result;
}

Vector3i Vector3iMultiplyValue(Vector3i v, int32_t val) {
	Vector3i result = { v.x*val, v.y*val, v.z*val };
	return result;
}

Vector3i Vector3iCrossProduct(Vector3i v1, Vector3i v2) {
	Vector3i result = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
	return result;
}

Vector3i Vector3iPerpendicular(Vector3i v) {
	Vector3i result = { 0 };

	int32_t min = abs(v.x);
	Vector3i cardinalAxis = {1, 0, 0};

	if (abs(v.y) < min)
	{
		min = abs(v.y);
		Vector3i tmp = {0, 1, 0};
		cardinalAxis = tmp;
	}

	if (abs(v.z) < min)
	{
		Vector3i tmp = {0, 0, 1};
		cardinalAxis = tmp;
	}

	// Cross product between vectors
	result.x = v.y*cardinalAxis.z - v.z*cardinalAxis.y;
	result.y = v.z*cardinalAxis.x - v.x*cardinalAxis.z;
	result.z = v.x*cardinalAxis.y - v.y*cardinalAxis.x;

	return result;
}

float Vector3iLength(const Vector3i v) {
	float result = sqrtf((float)(v.x*v.x + v.y*v.y + v.z*v.z));
	return result;
}

int32_t Vector3iLengthSqr(const Vector3i v) {
	int32_t result = v.x*v.x + v.y*v.y + v.z*v.z;
	return result;
}

int32_t Vector3iDotProduct(Vector3i v1, Vector3i v2) {
	int32_t result = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
	return result;
}

float Vector3iDistance(Vector3i v1, Vector3i v2) {
	float result = 0.0f;

	int32_t dx = v2.x - v1.x;
	int32_t dy = v2.y - v1.y;
	int32_t dz = v2.z - v1.z;
	result = sqrtf((float)(dx*dx + dy*dy + dz*dz));

	return result;
}

int32_t Vector3iDistanceSqr(Vector3i v1, Vector3i v2) {
	int32_t result = 0;

	int32_t dx = v2.x - v1.x;
	int32_t dy = v2.y - v1.y;
	int32_t dz = v2.z - v1.z;
	result = dx*dx + dy*dy + dz*dz;

	return result;
}

float Vector3iAngle(Vector3i v1, Vector3i v2) {
	float result = 0.0f;

	Vector3i cross = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
	float len = sqrtf((float)(cross.x*cross.x + cross.y*cross.y + cross.z*cross.z));
	float dot = (float)(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
	result = atan2f(len, dot);

	return result;
}

Vector3i Vector3iNegate(Vector3i v) {
	Vector3i result = { -v.x, -v.y, -v.z };
	return result;
}

Vector3i Vector3iDivide(Vector3i v1, Vector3i v2) {
	Vector3i result = { v1.x/v2.x, v1.y/v2.y, v1.z/v2.z };
	return result;
}

Vector3i Vector3iDivideValue(Vector3i v, int32_t val) {
	Vector3i result = { v.x/val, v.y/val, v.z/val };
	return result;
}

Vector3i Vector3iReflect(Vector3i v, Vector3i normal) {
	Vector3i result = { 0 };

	// I is the original vector
	// N is the normal of the incident plane
	// R = I - (2*N*(DotProduct[I, N]))

	int32_t dotProduct = (v.x*normal.x + v.y*normal.y + v.z*normal.z);

	result.x = v.x - (2*normal.x)*dotProduct;
	result.y = v.y - (2*normal.y)*dotProduct;
	result.z = v.z - (2*normal.z)*dotProduct;

	return result;
}

Vector3i Vector3iMin(Vector3i v1, Vector3i v2) {
	Vector3i result = { 0 };

	result.x = imin(v1.x, v2.x);
	result.y = imin(v1.y, v2.y);
	result.z = imin(v1.z, v2.z);

	return result;
}

Vector3i Vector3iMax(Vector3i v1, Vector3i v2) {
	Vector3i result = { 0 };

	result.x = imax(v1.x, v2.x);
	result.y = imax(v1.y, v2.y);
	result.z = imax(v1.z, v2.z);

	return result;
}

int3 Vector3iToIntV(Vector3i v) {
	int3 buffer = { 0 };

	buffer.v[0] = v.x;
	buffer.v[1] = v.y;
	buffer.v[2] = v.z;

	return buffer;
}

Vector3i Vector3iClamp(Vector3i v, Vector3i min, Vector3i max) {
	Vector3i result = { 0 };

	result.x = imin(max.x, imax(min.x, v.x));
	result.y = imin(max.y, imax(min.y, v.y));
	result.z = imin(max.z, imax(min.z, v.z));

	return result;
}

int Vector3iEquals(Vector3i p, Vector3i q) {
	int result = (p.x == q.x) && (p.y == q.y) && (p.z == q.z);
	return result;
}

Vector4i Vector4iAdd(Vector4i q1, Vector4i q2) {
	Vector4i result = {q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w};
	return result;
}

Vector4i Vector4iAddValue(Vector4i q, int32_t add) {
	Vector4i result = {q.x + add, q.y + add, q.z + add, q.w + add};
	return result;
}

Vector4i Vector4iSubtract(Vector4i q1, Vector4i q2) {
	Vector4i result = {q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w};
	return result;
}

Vector4i Vector4iSubtractValue(Vector4i q, int32_t sub) {
	Vector4i result = {q.x - sub, q.y - sub, q.z - sub, q.w - sub};
	return result;
}

Vector4i Vector4iIdentity(void) {
	Vector4i result = { 0, 0, 0, 1 };
	return result;
}

float Vector4iLength(Vector4i q) {
	float result = sqrtf((float)(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w));
	return result;
}

Vector4i Vector4iMultiply(Vector4i q1, Vector4i q2) {
	Vector4i result = { 0 };

	int32_t qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
	int32_t qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;

	result.x = qax*qbw + qaw*qbx + qay*qbz - qaz*qby;
	result.y = qay*qbw + qaw*qby + qaz*qbx - qax*qbz;
	result.z = qaz*qbw + qaw*qbz + qax*qby - qay*qbx;
	result.w = qaw*qbw - qax*qbx - qay*qby - qaz*qbz;

	return result;
}

Vector4i Vector4iMultiplyValue(Vector4i q, int32_t val) {
	Vector4i result = { q.x*val, q.y*val, q.z*val, q.w*val };
	return result;
}

Vector4i Vector4iDivide(Vector4i q1, Vector4i q2) {
	Vector4i result = { q1.x/q2.x, q1.y/q2.y, q1.z/q2.z, q1.w/q2.w };
	return result;
}

Vector4i Vector4iDivideValue(Vector4i q, int32_t val) {
	Vector4i result = { q.x/val, q.y/val, q.z/val, q.w/val };
	return result;
}

Vector4i Vector4iNegate(Vector4i q) {
	Vector4i result = { -q.x, -q.y, -q.z, -q.w };
	return result;
}

#define _LUNA_ID_START 100000000u
#define _LUNA_ID_END   999999999u
#define _LUNA_ID_LIMIT (_LUNA_ID_END - _LUNA_ID_START)

static _LUNA_ID_TYPE _pbs_id_seed = ID_NULL + 1;

_LUNA_ID_TYPE _luna_id_generate() {
	_pbs_id_seed = _pbs_id_seed * 0x343FD + 0x269EC3;
	_LUNA_ID_TYPE r = (_pbs_id_seed >> 0x10) & 0x7FFF;
	return (r % _LUNA_ID_LIMIT) + _LUNA_ID_START;
}
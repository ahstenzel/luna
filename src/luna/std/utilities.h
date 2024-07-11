#ifndef LUNA_STD_UTILITIES_H
#define LUNA_STD_UTILITIES_H
/**
 * utilities.h
 * Useful global utility functions.
*/
#include "luna/platform.h"
#include "raylib/raymath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

//----------------------------------------------------------------------------------
// Functional macros
//----------------------------------------------------------------------------------

/// @brief Get the number of elements in the buffer.
#define LUNA_ARR_LEN(x) (sizeof(x)/sizeof(x[0]))

/// @brief Get the next power of 2 >= x.
#define LUNA_NEXT_POW2(x) (1ULL << (unsigned long long)(log2((double)x-1.)+1.))

#if defined(LUNA_CMP_GCC) || defined(LUNA_CMP_CLANG)

/// @brief Get the larger of two values.
#define LUNA_MAX(a,b) \
({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
})

/// @brief Get the smaller of two values.
#define LUNA_MIN(a,b) \
({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
})

#else

/// @brief Get the larger of two values.
#define LUNA_MAX(a,b) (((a) > (b)) ? a : b)

/// @brief Get the smaller of two values.
#define LUNA_MIN(a,b) (((a) < (b)) ? a : b)

#endif // defined(LUNA_CMP_GCC) || defined(LUNA_CMP_CLANG)

// Unique ID generation
#define _LUNA_ID_TYPE unsigned int	// Base type for all IDs.
#define ID_NULL 0				    // Signifier for an invalid ID.

/// @brief Generate a unique ID vaule.
/// @return ID
_LUNA_ID_TYPE _luna_id_generate();


//----------------------------------------------------------------------------------
// BAMS Functions
//----------------------------------------------------------------------------------

typedef short Angle;	// Angle units stored as BAM (Binary angular measurement) integers.

#define ANGLE_MIN 0x0000
#define ANGLE_MAX 0xFFFF
#define ANGLE_HALF 0x7FFF

/// @brief Convert the angle value (BAM integer) to degrees (float).
#define ANGLE_TO_DEG(a) ((float)(a) * 360.f / (float)(ANGLE_MAX))

/// @brief Convert the degrees value (float) to angle units (BAM integer).
#define DEG_TO_ANGLE(a) (Angle)(a * (float)(ANGLE_MAX) / 360.f )

/// @brief Convert the angle value (BAM integer) to radians (float).
#define ANGLE_TO_RAD(a) ((float)(a) * (2.f * PI) / (float)(ANGLE_MAX))

/// @brief Convert the radians value (float) to angle units (BAM integer).
#define RAD_TO_ANGLE(a) (Angle)(a * (float)(ANGLE_MAX) / (2.f * PI) )


//----------------------------------------------------------------------------------
// Extra Vector Operations
//----------------------------------------------------------------------------------

/// @brief Rotate the point v1 around the point v2 by the given angle
/// @param v1 First vector
/// @param v2 Second vector
/// @param angle Angle of rotation (BAM integers)
/// @return Rotated vector
Vector2 Vector2RotatePoint(Vector2 v1, Vector2 v2, Angle angle);

/// @brief Reflect the vector v1 across v2, along the line between them
/// @param v1 First vector
/// @param v2 Second vector
/// @return Reflected vector
Vector2 Vector2ReflectPoint(Vector2 v1, Vector2 v2);

//----------------------------------------------------------------------------------
// Integer Vectors
//----------------------------------------------------------------------------------

typedef struct {
	int32_t x;
	int32_t y;
} Vector2i;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} Vector3i;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;
} Vector4i;

typedef struct {
	int32_t v[3];
} int3;

// Get int vector for Vector3i
#define Vector3iToInt(vec) (Vector3iToIntV(vec).v)

//----------------------------------------------------------------------------------
// Vector2i math
//----------------------------------------------------------------------------------

// Vector with components value 0
Vector2i Vector2iZero(void);

// Vector with components value 1
Vector2i Vector2iOne(void);

// Add two vectors (v1 + v2)
Vector2i Vector2iAdd(Vector2i v1, Vector2i v2);

// Add vector and int value
Vector2i Vector2iAddValue(Vector2i v, int32_t add);

// Subtract two vectors (v1 - v2)
Vector2i Vector2iSubtract(Vector2i v1, Vector2i v2);

// Subtract vector by int value
Vector2i Vector2iSubtractValue(Vector2i v, int32_t sub);

// Calculate vector length
float Vector2iLength(Vector2i v);

// Calculate vector square length
int32_t Vector2iLengthSqr(Vector2i v);

// Calculate two vectors dot product
int32_t Vector2iDotProduct(Vector2i v1, Vector2i v2);

// Calculate distance between two vectors
float Vector2iDistance(Vector2i v1, Vector2i v2);

// Calculate square distance between two vectors
int32_t Vector2iDistanceSqr(Vector2i v1, Vector2i v2);

// Calculate angle between two vectors
// NOTE: Angle is calculated from origin point (0, 0)
float Vector2iAngle(Vector2i v1, Vector2i v2);

// Multiply vector by vector
Vector2i Vector2iMultiply(Vector2i v1, Vector2i v2);

// Multiply each component by value
Vector2i Vector2iMultiplyValue(Vector2i v, int32_t val);

// Negate vector
Vector2i Vector2iNegate(Vector2i v);

// Divide vector by vector
Vector2i Vector2iDivide(Vector2i v1, Vector2i v2);

// Divide each component by value
Vector2i Vector2iDivideValue(Vector2i v, int32_t val);

// Calculate reflected vector to normal
Vector2i Vector2iReflect(Vector2i v, Vector2i normal);

// Clamp the components of the vector between
// min and max values specified by the given vectors
Vector2i Vector2iClamp(Vector2i v, Vector2i min, Vector2i max);

// Check whether two given vectors are almost equal
int Vector2iEquals(Vector2i p, Vector2i q);

// Cast an integer vector to a float vector.
Vector2 Vector2iCastFloat(Vector2i v);

//----------------------------------------------------------------------------------
// Vector3i math
//----------------------------------------------------------------------------------

// Vector with components value 0
Vector3i Vector3iZero(void);

// Vector with components value 1
Vector3i Vector3iOne(void);

// Add two vectors
Vector3i Vector3iAdd(Vector3i v1, Vector3i v2);

// Add vector and int value
Vector3i Vector3iAddValue(Vector3i v, int32_t add);

// Subtract two vectors
Vector3i Vector3iSubtract(Vector3i v1, Vector3i v2);

// Subtract vector by int value
Vector3i Vector3iSubtractValue(Vector3i v, int32_t sub);

// Multiply vector by vector
Vector3i Vector3iMultiply(Vector3i v1, Vector3i v2);

// Multiply each component by value
Vector3i Vector3iMultiplyValue(Vector3i v, int32_t value);

// Calculate two vectors cross product
Vector3i Vector3iCrossProduct(Vector3i v1, Vector3i v2);

// Calculate one vector perpendicular vector
Vector3i Vector3iPerpendicular(Vector3i v);

// Calculate vector length
float Vector3iLength(const Vector3i v);

// Calculate vector square length
int32_t Vector3iLengthSqr(const Vector3i v);

// Calculate two vectors dot product
int32_t Vector3iDotProduct(Vector3i v1, Vector3i v2);

// Calculate distance between two vectors
float Vector3iDistance(Vector3i v1, Vector3i v2);

// Calculate square distance between two vectors
int32_t Vector3iDistanceSqr(Vector3i v1, Vector3i v2);

// Calculate angle between two vectors
float Vector3iAngle(Vector3i v1, Vector3i v2);

// Negate provided vector (invert direction)
Vector3i Vector3iNegate(Vector3i v);

// Divide vector by vector
Vector3i Vector3iDivide(Vector3i v1, Vector3i v2);

// Divide each component by value
Vector3i Vector3iDivideValue(Vector3i v, int32_t val);

// Calculate reflected vector to normal
Vector3i Vector3iReflect(Vector3i v, Vector3i normal);

// Get min value for each pair of components
Vector3i Vector3iMin(Vector3i v1, Vector3i v2);

// Get max value for each pair of components
Vector3i Vector3iMax(Vector3i v1, Vector3i v2);

// Get Vector3i as int array
int3 Vector3iToIntV(Vector3i v);

// Clamp the components of the vector between
// min and max values specified by the given vectors
Vector3i Vector3iClamp(Vector3i v, Vector3i min, Vector3i max);

// Check whether two given vectors are almost equal
int Vector3iEquals(Vector3i p, Vector3i q);

// Cast an integer vector to a float vector.
Vector3 Vector3iCastFloat(Vector3i v);

//----------------------------------------------------------------------------------
// Vector4i math
//----------------------------------------------------------------------------------

// Add two vectors
Vector4i Vector4iAdd(Vector4i q1, Vector4i q2);

// Add Vector4i and int value
Vector4i Vector4iAddValue(Vector4i q, int32_t add);

// Subtract two Vector4is
Vector4i Vector4iSubtract(Vector4i q1, Vector4i q2);

// Subtract Vector4i and int value
Vector4i Vector4iSubtractValue(Vector4i q, int32_t sub);

// Get identity Vector4i
Vector4i Vector4iIdentity(void);

// Computes the length of a Vector4i
float Vector4iLength(Vector4i q);

// Calculate two Vector4i multiplication
Vector4i Vector4iMultiply(Vector4i q1, Vector4i q2);

// Multiply each component by value
Vector4i Vector4iMultiplyValue(Vector4i q, int32_t val);

// Divide two Vector4is
Vector4i Vector4iDivide(Vector4i q1, Vector4i q2);

// Divide each component by value
Vector4i Vector4iDivideValue(Vector4i q, int32_t val);

// Negate provided vector (invert direction)
Vector4i Vector4iNegate(Vector4i q);

// Cast an integer vector to a float vector.
Vector4 Vector4iCastFloat(Vector4i v);

#define VECTOR2_IS_ZERO(v) ((v).x == 0.f && (v).y == 0.f)

#define VECTOR3_IS_ZERO(v) ((v).x == 0.f && (v).y == 0.f && (v).z == 0.f)

#define VECTOR4_IS_ZERO(v) ((v).x == 0.f && (v).y == 0.f && (v).z == 0.f && (v).w == 0.f)

#define VECTOR2I_IS_ZERO(v) ((v).x == 0 && (v).y == 0)

#define VECTOR3I_IS_ZERO(v) ((v).x == 0 && (v).y == 0 && (v).z == 0)

#define VECTOR4I_IS_ZERO(v) ((v).x == 0 && (v).y == 0 && (v).z == 0 && (v).w == 0)

#define VECTOR2_ANY_ZERO(v) ((v).x == 0.f || (v).y == 0.f)

#define VECTOR3_ANY_ZERO(v) ((v).x == 0.f || (v).y == 0.f || (v).z == 0.f)

#define VECTOR4_ANY_ZERO(v) ((v).x == 0.f || (v).y == 0.f || (v).z == 0.f || (v).w == 0.f)

#define VECTOR2I_ANY_ZERO(v) ((v).x == 0 || (v).y == 0)

#define VECTOR3I_ANY_ZERO(v) ((v).x == 0 || (v).y == 0 || (v).z == 0)

#define VECTOR4I_ANY_ZERO(v) ((v).x == 0 || (v).y == 0 || (v).z == 0 || (v).w == 0)

#endif // LUNA_STD_UTILITIES_H
#ifndef LUNA_STD_VECTOR_H
#define LUNA_STD_VECTOR_H
/**
 * vector.h
 * Dynamically resizing array.
*/
#include "luna/std/utilities.h"

#ifndef VECTOR_DEFAULT_CAPACITY
#define VECTOR_DEFAULT_CAPACITY 1ULL
#endif
#ifndef VECTOR_MAX_CAPACITY
#define VECTOR_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _vec_pos(v, i) &(v)->_buffer[0] + ((i) * (v)->_element_size)

/// @brief Create a new vector.
/// @param t Vector type
/// @return Vector pointer
#define vector_create(t) _vec_factory(sizeof(t), VECTOR_DEFAULT_CAPACITY)

/// @brief Deallocate a vector.
/// @param v Vector pointer
#define vector_destroy(v) free(v)

/// @brief Get an element from the vector.
/// @param v Vector pointer
/// @param i Index
/// @return Void data pointer, or NULL if out of range
#define vector_get(v, i) (void*)((i < (v)->_length && i >= 0) ? _vec_pos(v, i) : NULL)

/// @brief Get the first element in the vector.
/// @param v Vecor pointer
/// @return Void data pointer, or NULL if empty
#define vector_get_front(v) (void*)(((v)->_length > 0) ? _vec_pos(v, 0) : NULL)

/// @brief Get the last element in the vector.
/// @param v Vector pointer
/// @return Void data pointer, or NULL if empty
#define vector_get_back(v) (void*)(((v)->_length > 0) ? _vec_pos(v, (v)->_length-1) : NULL)

/// @brief Set the element in the vector.
/// @param v Vector pointer
/// @param i Index
/// @param d Data pointer
#define vector_set(v, i, d) \
({ \
	if (i < (v)->_length && i >= 0) memcpy_s(_vec_pos(v, i), (v)->_element_size, d, (v)->_element_size); \
})

/// @brief Get the number of elements in the vector.
/// @param v Vector pointer
/// @return Vector size
#define vector_size(v) ((v)->_length)

/// @brief Add an element to the end of the vector.
/// @param v Vector pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define vector_push_back(v, d) _vec_insert(&v, (v)->_length, (void*)d)

/// @brief Add an element to the front of the vector.
/// @param v Vector pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define vector_push_front(v, d) _vec_insert(&v, 0, (void*)d)

/// @brief Remove the element at the end of the vector.
/// @param v Vector pointer
#define vector_pop_back(v) _vec_remove(v, (v)->_length - 1, 1)

/// @brief Remove the element at the front of the vector.
/// @param v Vector pointer
#define vector_pop_front(v) _vec_remove(v, 0, 1)

/// @brief Insert the element at the given point in the vector.
/// @param v Vector pointer
/// @param i Index
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define vector_insert(v, i, d) _vec_insert(v, i, (void*)d)

/// @brief Remove the element at the given point in the vector.
/// @param v Vector pointer
/// @param i Index
#define vector_remove(v, i) _vec_remove(v, i, 1)

/// @brief Remove all elements from the vector.
/// @param v Vector pointer
#define vector_clear(v) _vec_remove(v, 0, (v)->_length)

/// @brief Get the size of the vector in memory.
/// @param v Vector pointer
/// @return Number of bytes
#define vector_bytes(v) ((v) ? (_vec_size((v)->_element_size, (v)->_capacity)) : 0)

/// @brief Swap the contents of two positions in the vector.
/// @param v Vector pointer
/// @param a First index
/// @param b Second index
#define vector_swap(v, a, b) _vec_swap(v, a, b)

/// @brief Dynamically resizing array.
typedef struct {
	size_t _length;
	size_t _capacity;
	size_t _element_size;
	uint8_t _buffer[];
} vector_t;

size_t _vec_size(size_t, size_t);

vector_t* _vec_factory(size_t, size_t);

vector_t* _vec_resize(vector_t*, size_t);

void* _vec_insert(vector_t**, size_t, void*);

void _vec_remove(vector_t*, size_t, size_t);

void _vec_swap(vector_t*, size_t, size_t);

#endif  // LUNA_STD_VECTOR_H
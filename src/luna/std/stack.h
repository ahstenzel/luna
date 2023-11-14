#ifndef LUNA_STD_STACK_H
#define LUNA_STD_STACK_H
/**
 * stack.h
 * LIFO group of elements.
*/
#include "luna/std/utilities.h"

#ifndef STACK_DEFAULT_CAPACITY
#define STACK_DEFAULT_CAPACITY 1ULL
#endif
#ifndef STACK_MAX_CAPACITY
#define STACK_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _stack_pos(s, i) &(s)->_buffer[0] + ((i) * (s)->_element_size)

/// @brief Create a new stack.
/// @param t Stack type
/// @return Stack pointer
#define stack_create(t) _stack_factory(sizeof(t), STACK_DEFAULT_CAPACITY)

/// @brief Deallocate a stack.
/// @param s Stack pointer
#define stack_destroy(s) free(s)

/// @brief Get the top element of the stack.
/// @param s Stack pointer
/// @return Void data pointer, or NULL if empty
#define stack_head(s) (void*)((s)->_length > 0 ? _stack_pos(s, (s)->_length - 1) : NULL)

/// @brief Add a new element to the top of the stack.
/// @param s Stack pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define stack_push(s, d) _stack_insert(&s, (void*)d)

/// @brief Remove the element from the top of the stack.
/// @param s Stack pointer
#define stack_pop(s) _stack_remove(s, 1)

/// @brief Get the number of elements in the stack.
/// @param s Stack pointer
/// @return Stack size
#define stack_size(s) ((s)->_length)

/// @brief Remove all elements from the stack.
/// @param s Stack pointer
#define stack_clear(s) _stack_remove(s, (s)->_length)

/// @brief Get the size of the stack in memory.
/// @param s Stack pointer
/// @return Number of bytes
#define stack_bytes(s) ((s) ? (_stack_size((s)->_element_size, (s)->_capacity)) : 0)

/// @brief LIFO group of elements.
typedef struct {
	size_t _length;
	size_t _capacity;
	size_t _element_size;
	uint8_t _buffer[];
} stack_t;

size_t _stack_size(size_t, size_t);

stack_t* _stack_factory(size_t, size_t);

stack_t* _stack_resize(stack_t*, size_t);

void* _stack_insert(stack_t**, void*);

void _stack_remove(stack_t*, size_t);

#endif  // LUNA_STD_STACK_H
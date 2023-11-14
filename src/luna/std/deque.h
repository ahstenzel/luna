#ifndef LUNA_STD_DEQUEUE_H
#define LUNA_STD_DEQUEUE_H
/*
 * deque.h
 * Double-ended queue.
 */
#include "luna/std/utilities.h"

#ifndef DEQUE_DEFAULT_CAPACITY
#define DEQUE_DEFAULT_CAPACITY 1ULL
#endif
#ifndef DEQUE_MAX_CAPACITY
#define DEQUE_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _deque_pos(q, i) &(q)->_buffer[0] + ((i) * (q)->_element_size)

/// @brief Create a new deque.
/// @param t Dequeue type
/// @return Dequeue pointer
#define deque_create(t) _deque_factory(sizeof(t), DEQUE_DEFAULT_CAPACITY)

/// @brief Deallocate a deque.
/// @param q Dequeue pointer.
#define deque_destroy(q) free(q)

/// @brief Get the front element of the deque.
/// @param q Dequeue pointer
/// @return Void data pointer, or NULL if empty
#define deque_front(q) (void*)((q)->_length > 0 ? _deque_pos(q, (q)->_tail) : NULL)

/// @brief Get the back element of the deque.
/// @param q Dequeue pointer
/// @return Void data pointer, or NULL if empty
#define deque_back(q) (void*)((q)->_length > 0 ? _deque_pos(q, (q)->_head) : NULL)

/// @brief Add an element to the front of the deque.
/// @param q Dequeue pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define deque_push_front(q, d) _deque_insert_front(&q, (void*)d)

/// @brief Add an element to the back of the deque.
/// @param q Dequeue pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define deque_push_back(q, d) _deque_insert_back(&q, (void*)d)

/// @brief Remove the element at the front of the deque.
/// @param q Dequeue pointer
#define deque_pop_front(q) _deque_remove_front(q, 1)

/// @brief Remove the element at the back of the deque.
/// @param q Dequeue pointer
#define deque_pop_back(q) _deque_remove_back(q, 1)

/// @brief Get the number of elements in the deque.
/// @param q Dequeue pointer
/// @return Dequeue size
#define deque_size(q) ((q)->_length)

/// @brief Remove all elements from the deque.
/// @param q Dequeue pointer
#define deque_clear(q) _deque_remove(q, (q)->_length)

/// @brief Get the sze of the deque in memory.
/// @param q Dequeue pointer
/// @return Number of bytes
#define deque_bytes(q) ((q) ? (_deque_size((q)->_element_size, (q)->_capacity)) : 0)

/// @brief Double-ended queue.
typedef struct {
	size_t _length;
	size_t _head;
	size_t _tail;
	size_t _capacity;
	size_t _element_size;
	uint8_t _buffer[];
} deque_t;

size_t _deque_size(size_t, size_t);

deque_t* _deque_factory(size_t, size_t);

deque_t* _deque_resize(deque_t*, size_t);

void* _deque_insert_front(deque_t**, void*);

void* _deque_insert_back(deque_t**, void*);

void _deque_remove_front(deque_t*, size_t);

void _deque_remove_back(deque_t*, size_t);

#endif	// LUNA_STD_DEQUEUE_H
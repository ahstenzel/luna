#ifndef LUNA_STD_QUEUE_H
#define LUNA_STD_QUEUE_H
/**
 * queue.h
 * FIFO group of elements.
*/
#include "luna/std/utilities.h"

#ifndef QUEUE_DEFAULT_CAPACITY
#define QUEUE_DEFAULT_CAPACITY 1ULL
#endif
#ifndef QUEUE_MAX_CAPACITY
#define QUEUE_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _queue_pos(q, i) &(q)->_buffer[0] + ((i) * (q)->_element_size)

/// @brief Create a new queue.
/// @param t Queue type
/// @return Queue pointer
#define queue_create(t) _queue_factory(sizeof(t), QUEUE_DEFAULT_CAPACITY)

/// @brief Deallocate a queue.
/// @param q Queue pointer
#define queue_destroy(q) free(q)

/// @brief Get the front element of the queue.
/// @param q Queue pointer
/// @return Void data pointer, or NULL if empty
#define queue_head(q) (void*)((q)->_length > 0 ? _queue_pos(q, (q)->_head) : NULL)

/// @brief Add a new element to the back of the queue.
/// @param q Queue pointer
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define queue_push(q, d) _queue_insert(&q, (void*)d)

/// @brief Remove the element at the front of the queue.
/// @param q Queue pointer
#define queue_pop(q) _queue_remove(q, 1)

/// @brief Get the number of elements in the queue.
/// @param q Queue pointer
/// @return Queue size
#define queue_size(q) ((q)->_length)

/// @brief Remove all elements from the queue.
/// @param q Queue pointer
#define queue_clear(q) _queue_remove(q, (q)->_length)

/// @brief Get the size of the queue in memory.
/// @param q Queue pointer
/// @return Number of bytes
#define queue_bytes(q) ((q) ? (_queue_size((q)->_element_size, (q)->_capacity)) : 0)

/// @brief FIFO group of elements.
typedef struct {
	size_t _length;
	size_t _head;
	size_t _tail;
	size_t _capacity;
	size_t _element_size;
	uint8_t _buffer[];
} queue_t;

size_t _queue_size(size_t, size_t);

queue_t* _queue_factory(size_t, size_t);

queue_t* _queue_resize(queue_t*, size_t);

void* _queue_insert(queue_t**, void*);

void _queue_remove(queue_t*, size_t);

#endif	// LUNA_STD_QUEUE_H
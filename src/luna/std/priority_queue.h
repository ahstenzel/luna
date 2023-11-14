#ifndef LUNA_STD_PRIORITY_QUEUE_H
#define LUNA_STD_PRIORITY_QUEUE_H
/**
 * priority_queue.h
 * Sorted queue of value-data pairs.
*/
#include "luna/std/utilities.h"

typedef int32_t priority_queue_value_t;

#ifndef PRIORITY_QUEUE_DEFAULT_CAPACITY
#define PRIORITY_QUEUE_DEFAULT_CAPACITY 1ULL
#endif
#ifndef PRIORITY_QUEUE_MAX_CAPACITY
#define PRIORITY_QUEUE_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _priority_queue_value_pos(q, i) &(q)->_buffer[0] + ((i) * sizeof(priority_queue_value_t))
#define _priority_queue_data_pos(q, i) &(q)->_buffer[0] + ((q)->_capacity * sizeof(priority_queue_value_t)) + ((i) * (q)->_element_size)
#define _priority_queue_value(q, i) *(priority_queue_value_t*)(_priority_queue_value_pos(q, i))

/// @brief Create a new priority queue.
/// @param t Priority queue type
/// @return Priority queue pointer
#define priority_queue_create(t) _priority_queue_factory(sizeof(t), PRIORITY_QUEUE_DEFAULT_CAPACITY)

/// @brief Deallocate a priority queue.
/// @param q Priority queue pointer
#define priority_queue_destroy(q) free(q)

/// @brief Get the top element in the priority queue.
/// @param q Priority queue pointer
/// @param t Priority queue type
/// @return Void data pointer, or NULL if empty
#define priority_queue_top_data(q) (void*)((q)->_length > 0 ? _priority_queue_data_pos(q, (q)->_length - 1) : NULL)

/// @brief Get the value of the top element in the priority queue.
/// @param q Priority queue pointer
/// @return Value pointer, or NULL if empty
#define priority_queue_top_value(q) (priority_queue_value_t*)((q)->_length > 0 ? _priority_queue_value_pos(q, (q)->_length - 1) : NULL)

/// @brief Add an element to the priority queue.
/// @param q Priority queue pointer
/// @param v Priority value
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL on failure
#define priority_queue_push(q, v, d) _priority_queue_insert(&q, v, (void*)d)

/// @brief Remove the top element from the priority queue.
/// @param q Priority queue pointer
#define priority_queue_pop(q) _priority_queue_remove(q, 1)

/// @brief Remove an element with the given value from the priority queue. If the provided data is NULL, find the first element matching the given
/// @brief priority value. Otherwise, find the element which matches the given data exactly.
/// @param q Priority queue pointer
/// @param v Priority value
/// @param d Data pointer
#define priority_queue_remove(q, v, d) _priority_queue_remove_value(q, v, d);

/// @brief Get the number of elements in the priority queue.
/// @param q Priority queue pointer
/// @return Number of elements
#define priority_queue_size(q) ((q)->_length)

/// @brief Remove all elements in the priority queue.
/// @param q Priority queue pointer
#define priority_queue_clear(q) _priority_queue_remove(q, (q)->_length)

/// @brief Get the size of the priority queue in memory.
/// @param q Priority queue pointer
/// @return Number of bytes
#define priority_queue_bytes(q) ((q) ? (_priority_queue_size((q)->_element_size, (q)->_capacity)) : 0)

/// @brief Find the data in the priority queue if it exists. If the provided data is NULL, find the first element matching the given priority
/// @brief value. Otherwise, find the element which matches the given data exactly.
/// @param q Priority queue pointer
/// @param v Priority value
/// @param d Data pointer (optional)
/// @return Data pointer, or NULL if not found
#define priority_queue_find(q, v, d) _priority_queue_find(q, v, d)

/// @brief Create an iterator for the priority queue.
/// @param q Priority queue pointer
/// @return Iterator pointer
#define priority_queue_it(q) _priority_queue_it(q)

/// @brief Create an iterator for the priority queue starting at the given value.
/// @param q Priority queue pointer
/// @param v Priority value
/// @return Iterator pointer
#define priority_queue_it_value(q, v) _priority_queue_it_value(q, v)

/// @brief Move the iterator to the next element.
/// @param i Iterator pointer
#define priority_queue_it_next(i) _priority_queue_it_next(&i)

/// @brief List of elements sorted by priority.
typedef struct {
	size_t _length;
	size_t _capacity;
	size_t _element_size;
	uint8_t _buffer[];
} priority_queue_t;

/// @brief Iterator for a priority queue.
typedef struct {
	priority_queue_t* _qu;
	void* data;
	size_t _index;
	priority_queue_value_t value;
} priority_queue_it_t;

size_t _priority_queue_size(size_t, size_t);

priority_queue_t* _priority_queue_factory(size_t, size_t);

priority_queue_t* _priority_queue_resize(priority_queue_t*, size_t);

void* _priority_queue_insert(priority_queue_t**, priority_queue_value_t, void*);

void _priority_queue_remove(priority_queue_t*, size_t);

void _priority_queue_remove_value(priority_queue_t*, priority_queue_value_t, void*);

void _priority_queue_sort(priority_queue_t*);

size_t _priority_queue_find_index(priority_queue_t*, priority_queue_value_t, void*);

void* _priority_queue_find(priority_queue_t*, priority_queue_value_t, void*);

priority_queue_it_t* _priority_queue_it(priority_queue_t*);

priority_queue_it_t* _priority_queue_it_value(priority_queue_t*, priority_queue_value_t);

void _priority_queue_it_next(priority_queue_it_t**);

#endif	// LUNA_STD_PRIORITY_QUEUE_H
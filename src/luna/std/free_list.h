#ifndef LUNA_STD_FREE_LIST_H
#define LUNA_STD_FREE_LIST_H
/**
 * free_list.h
 * List of elements that fills in empty slots first.
*/
#include "luna/std/utilities.h"

#ifndef FREE_LIST_DEFAULT_CAPACITY
#define FREE_LIST_DEFAULT_CAPACITY 8ULL
#endif
#ifndef FREE_LIST_MAX_CAPACITY
#define FREE_LIST_MAX_CAPACITY SIZE_MAX - 1
#endif

#define _free_list_bit_num(l) ((l->_capacity / 8) + 1)
#define _free_list_bit_set(l, n) (l->_buffer[n/8] |= (1 << (n % 8)))
#define _free_list_bit_clr(l, n) (l->_buffer[n/8] &= ~(1 << (n % 8)))
#define _free_list_bit_get(l, n) ((l->_buffer[n/8] >> (n % 8)) & 1)
#define _free_list_pos(l, i) &(l)->_buffer[_free_list_bit_num(l)] + ((i) * (l)->_element_size)

/// @brief Create a new free list.
/// @param t List type
/// @return List pointer
#define free_list_create(t) _free_list_factory(sizeof(t), FREE_LIST_DEFAULT_CAPACITY);

/// @brief Deallocate a list.
/// @param l List pointer
#define free_list_destroy(l) free(l);

/// @brief Get an element from the list.
/// @param l List pointer
/// @param i Index
/// @return Void data pointer, or NULL if invalid index
#define free_list_get(l, i) (void*)((i < (l)->_length && _free_list_bit_get(l, i)) ? _free_list_pos(l, i) : NULL)

/// @brief Get the number of elements in the list.
/// @param l List pointer
/// @return List size
#define free_list_size(l) ((l)->_length)

/// @brief Insert the element at the first free spot in the list.
/// @param l List pointer
/// @param i Index pointer (will be set to the index the element was inserted at, or NULL if insertion failed)
/// @param d Data pointer
/// @return Void data pointer to inserted element, or NULL if insertion failed
#define free_list_insert(l, i, d) _free_list_insert(&l, i, (void*)d)

/// @brief Remove the element at the point in the list.
/// @param l List pointer
/// @param i Index
#define free_list_remove(l, i) _free_list_remove(l, i, 1);

/// @brief Remove all elements from the list.
/// @param l List pointer
#define free_list_clear(l) _free_list_remove(l, 0, (l)->_length)

/// @brief Get the size of the list in memory.
/// @param l List pointer
/// @return Numberr of bytes
#define free_list_bytes(l) ((l) ? (offsetof(free_list_t, _buffer) + _free_list_buffer_size((l)->_element_size, (l)->_capacity)) : 0)

/// @brief Create an iterator for the list.
/// @param l List pointer
/// @return Iterator pointer
#define free_list_it(l) _free_list_it(l)

/// @brief Move the iterator to the next element.
/// @param i Iterator pointer
#define free_list_it_next(i) _free_list_it_next(&i)

/** Space-efficient list of elements. */
typedef struct {
	size_t _length;
	size_t _capacity;
	size_t _element_size;
	size_t _next_free;
	uint8_t _buffer[];
} free_list_t;

typedef struct {
	free_list_t* _list;
	void* data;
	size_t index;
} free_list_it_t;

size_t _free_list_buffer_size(size_t, size_t);

free_list_t* _free_list_factory(size_t, size_t);

free_list_t* _free_list_resize(free_list_t*, size_t);

void* _free_list_insert(free_list_t**, size_t*, void*);

void _free_list_remove(free_list_t*, size_t, size_t);

free_list_it_t* _free_list_it(free_list_t*);

void _free_list_it_next(free_list_it_t**);

#endif
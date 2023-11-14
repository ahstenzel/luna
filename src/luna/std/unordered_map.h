#ifndef LUNA_STD_UMAP_H
#define LUNA_STD_UMAP_H
/**
 * unordered_map.h
 * Hash table of key-value pairs.
 * Implemented as a simplified Swiss Table architecture.
*/
#include "luna/std/utilities.h"

typedef uint32_t _umap_key_t;
typedef uint32_t _umap_hash_t;
#define _umap_fnv_offset 2166136261U;
#define _umap_fnv_prime 16777619U;

#ifndef UMAP_DEFAULT_CAPACITY
#define UMAP_DEFAULT_CAPACITY 8ULL
#endif
#ifndef UMAP_MAX_CAPACITY
#define UMAP_MAX_CAPACITY SIZE_MAX - 1
#endif
#define _UMAP_DEFAULT_LOAD 0.875f
#define _UMAP_EMPTY 0x80     // 0b1000 0000
#define _UMAP_DELETED 0xFE   // 0b1111 1110
#define _UMAP_SENTINEL 0xFF  // 0b1111 1111

#define _umap_h1(h) h >> 7
#define _umap_h2(h) h & 0x7F
#define _umap_ctrl(u, i) (uint8_t*)(&(u)->_buffer[0] + i)
#define _umap_node(u, i) (&(u)->_buffer[0] + (u)->_capacity + (_umap_node_size((u)->_element_size) * i))
#define _umap_node_key(u, i) (_umap_key_t*)_umap_node(u, i)
#define _umap_node_data(u, i) (void*)(_umap_node(u, i) + ((u)->_element_size > sizeof(_umap_key_t) ? (u)->_element_size : sizeof(_umap_key_t)))

/// @brief Create a new unordered map.
/// @param t Map type
/// @return Map pointer
#define unordered_map_create(t) _umap_factory(sizeof(t), UMAP_DEFAULT_CAPACITY)

/// @brief Deallocate an unordered map.
/// @param u Map pointer
#define unordered_map_destroy(u) free(u)

/// @brief Add a new element to the map if it does not already exist.
/// @param u Map pointer
/// @param k Key
/// @param d Data pointer
/// @return Void data pointer to inserted element, or the already inserted element, or NULL on failure
#define unordered_map_insert(u, k, d) _umap_insert(&u, k, (void*)d)

/// @brief Find the element if it exists in the map.
/// @param u Map pointer
/// @param k Key
/// @return Void data pointer, or NULL if not found
#define unordered_map_find(u, k) _umap_find(u, k)

/// @brief Remove the element from the map.
/// @param u Map pointer
/// @param k Key
#define unordered_map_delete(u, k) _umap_delete(u, k)

/// @brief Get the number of elements in the map.
/// @param u Map pointer
/// @return Map size
#define unordered_map_size(u) ((u)->_length)

/// @brief Remove all elements from the map.
/// @param u Map pointer
#define unordered_map_clear(u) memset(_umap_ctrl(u, 0), _UMAP_EMPTY, (u)->_capacity)

/// @brief Create an iterator for the map.
/// @param u Map pointer
/// @return Iterator pointer
#define unordered_map_it(u) _umap_it(u)

/// @brief Move the iterator to the next element.
/// @param i Iterator pointer
#define unordered_map_it_next(i) _umap_it_next(&i)

/// @brief Get the size of the map in memory.
/// @param u Map pointer
#define unordered_map_bytes(u) ((u) ? (_umap_size((u)->_element_size, (u)->_capacity)) : 0)

/// @brief Hash table of key-value pairs.
typedef struct {
	size_t _length;
	size_t _capacity;
	size_t _element_size;
	size_t _load_count;
	uint8_t _buffer[];
} unordered_map_t;

/// @brief Iterator for an unordered map.
typedef struct {
	unordered_map_t* _umap;
	void* data;
	_umap_key_t key;
	size_t _index;
} unordered_map_it_t;

size_t _umap_node_size(size_t);

size_t _umap_size(size_t, size_t);

unordered_map_t* _umap_factory(size_t, size_t);

unordered_map_t* _umap_resize(unordered_map_t*, size_t);

_umap_hash_t _umap_hash(_umap_key_t);

void* _umap_insert(unordered_map_t**, _umap_key_t, void*);

void _umap_delete(unordered_map_t*, _umap_key_t);

void* _umap_find(unordered_map_t*, _umap_key_t);

unordered_map_it_t* _umap_it(unordered_map_t*);

void _umap_it_next(unordered_map_it_t**);

#endif  // LUNA_STD_UMAP_H
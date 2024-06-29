#include "luna/std/unordered_map_str.h"

size_t _umap_str_node_size(size_t element_size) {
	size_t key_size = sizeof(_umap_str_key_t);
	size_t size_max = LUNA_MAX(element_size, key_size);
	return (element_size + key_size + (size_max - 1)) & ~(size_max - 1);
}

size_t _umap_str_size(size_t element_size, size_t capacity) {
	size_t n = _umap_str_node_size(element_size);
	size_t c = n * capacity;
	if (c / capacity != n) { return 0; }
	if (c > SIZE_MAX - capacity) { return 0; }
	return LUNA_MAX(sizeof(unordered_map_str_t), offsetof(unordered_map_str_t, _buffer) + capacity + c);
}

unordered_map_str_t* _umap_str_factory(size_t element_size, size_t capacity) {
	size_t buffer_size = _umap_str_size(element_size, capacity);
	if (buffer_size == 0) { return NULL; }
	unordered_map_str_t* umap_str = calloc(1, buffer_size);
	if (!umap_str) { return NULL; }
	umap_str->_capacity = capacity;
	umap_str->_element_size = element_size;
	memset(_umap_str_ctrl(umap_str, 0), _UMAP_STR_EMPTY, capacity);
	return umap_str;
}

unordered_map_str_t* _umap_str_resize(unordered_map_str_t* umap_str, size_t new_capacity) {
	// Calculate new capacity
	if (new_capacity == 0) {
		size_t c = LUNA_NEXT_POW2(umap_str->_capacity + 1);
		new_capacity = LUNA_MIN(c, UMAP_STR_MAX_CAPACITY);
	}
	if (new_capacity > UMAP_STR_MAX_CAPACITY || new_capacity < umap_str->_length) { return NULL; }
	
	// Create new map
	unordered_map_str_t* new_umap_str = _umap_str_factory(umap_str->_element_size, new_capacity);
	if (!new_umap_str) { return NULL; }

	// Rehash data
	for (size_t i = 0; i < umap_str->_capacity; ++i) {
		uint8_t* ctrl = _umap_str_ctrl(umap_str, i);
		if (!((*ctrl) & _UMAP_STR_EMPTY)) {
			_umap_str_key_t _key = *_umap_str_node_key(umap_str, i);
			void* _data = _umap_str_node_data(umap_str, i);
			_umap_str_insert(&new_umap_str, _key, _data);
		}
	}

	// Return new map
	free(umap_str);
	return new_umap_str;
}

_umap_str_hash_t _umap_str_hash(_umap_str_key_t key) {
	uint8_t mask_size = sizeof(_umap_str_hash_t) - 4;
	_umap_str_hash_t hash = _umap_str_fnv_prime;
	_umap_str_hash_t mask = 0xF << mask_size;
	while (*key != '\0') {
		hash = (hash << 4) + *(key++);
		_umap_str_hash_t g = hash & mask;
		if (g != 0) hash ^= g >> mask_size;
		hash &= ~g;
	}
	return hash;
}

void* _umap_str_insert(unordered_map_str_t** umap_str, _umap_str_key_t key, void* data) {
	// Error check
	if (!umap_str || !(*umap_str)) { return NULL; }
	unordered_map_str_t* _umap_str = *umap_str;
	void* elem = _umap_str_find(_umap_str, key);
	if (elem) { return elem; }

	// Resize if needed
	if (_umap_str->_load_count / (float)_umap_str->_capacity >= _UMAP_STR_DEFAULT_LOAD) {
		unordered_map_str_t* temp = _umap_str_resize(_umap_str, 0);
		if (!temp) { return NULL; }
		(*umap_str) = temp;
		_umap_str = temp;
	}

	// Hash the key	
	_umap_str_hash_t h = _umap_str_hash(key);
	size_t pos = _umap_str_h1(h) & (_umap_str->_capacity - 1);

	// Linear probe to find an empty bucket
	while (1) {
		uint8_t* ctrl = _umap_str_ctrl(_umap_str, pos);
		// Space is empty if high bit is 1
		if ((*ctrl) & _UMAP_STR_EMPTY) {
			// Copy the key to a new buffer
			size_t dest_size = strlen(key) + 1;
			_umap_str_key_t dest = malloc(dest_size);
			if (!dest) { return NULL; }
			strcpy_s(dest, dest_size, key);

			// Save the key to the start of the node block
			//_umap_str_key_t* addr = _umap_str_node_key(_umap_str, pos);
			memcpy_s(_umap_str_node_key(_umap_str, pos), sizeof(_umap_str_key_t), &dest, sizeof(_umap_str_key_t));

			// Save lower 8 bits of hash to the control block
			_umap_str_hash_t h2 = _umap_str_h2(h);
			memcpy_s(ctrl, 1, &h2, 1);

			// Save the data to the end of the node block, aligned by the larger data type
			dest_size = _umap_str->_element_size;
			if (data) {
				memcpy_s(_umap_str_node_data(_umap_str, pos), dest_size, data, dest_size);
			}
			else {
				memset(_umap_str_node_data(_umap_str, pos), 0, dest_size);
			}
			break;
		}
		else {
			pos = (pos + 1) & (_umap_str->_capacity - 1);
		}
	}
	_umap_str->_length++;
	_umap_str->_load_count++;
	return _umap_str_node_data(_umap_str, pos);
}

void _umap_str_delete(unordered_map_str_t* umap_str, _umap_str_key_t key) {
	// Error check
	if (!umap_str) { return; }

	// Hash key again
	_umap_str_hash_t h = _umap_str_hash(key);
	size_t pos = _umap_str_h1(h) & (umap_str->_capacity - 1);

	// Linear probe to find key
	while (1) {
		uint8_t* ctrl = _umap_str_ctrl(umap_str, pos);
		// Check if this control byte matches lower byte of hash
		_umap_str_hash_t h2 = _umap_str_h2(h);
		if (*ctrl == h2) {
			// Verify key at this pos matches
			if (strcmp(*_umap_str_node_key(umap_str, pos), key) == 0) {
				memset(ctrl, _UMAP_STR_DELETED, 1);
				free(*_umap_str_node_key(umap_str, pos));
				umap_str->_length--;
				return;
			}
		}
		else if (*ctrl == _UMAP_STR_EMPTY) {
			// Empty slot marks the end of the bucket chain
			return;
		}
		else {
			// Look at next control byte
			pos = (pos + 1) & (umap_str->_capacity - 1);
		}
	}
}

void* _umap_str_find(unordered_map_str_t* umap_str, _umap_str_key_t key) {
	// Error check
	if (!umap_str) { return NULL; }

	// Hash key again
	_umap_str_hash_t h = _umap_str_hash(key);
	size_t pos = _umap_str_h1(h) & (umap_str->_capacity - 1);
	size_t first_pos = pos;

	// Linear probe to find key
	do {
		uint8_t* ctrl = _umap_str_ctrl(umap_str, pos);
		// Check if this control byte matches lower byte of hash
		_umap_str_hash_t h2 = _umap_str_h2(h);
		if (*ctrl == h2) {
			// Verify key at this pos matches
			if (strcmp(*_umap_str_node_key(umap_str, pos), key) == 0) {
				return _umap_str_node_data(umap_str, pos);
			}
		}
		else if (*ctrl == _UMAP_STR_EMPTY) {
			// Empty slot marks the end of the bucket chain
			return NULL;
		}
		else {
			// Look at next control byte
			pos = (pos + 1) & (umap_str->_capacity - 1);
		}
	} while (pos != first_pos);
	return NULL;
}

unordered_map_str_it_t* _umap_str_it(unordered_map_str_t* umap_str) {
	// Error check
	if (!umap_str || umap_str->_length == 0) { return NULL; }

	// Construct iterator
	size_t buffer_size = sizeof(unordered_map_str_it_t);
	unordered_map_str_it_t* it = malloc(buffer_size);
	if (!it) { return NULL; }
	memset(it, 0, buffer_size);
	it->_index = SIZE_MAX;
	it->_umap_str = umap_str;
	
	// Find first valid entry in map
	_umap_str_it_next(&it);
	return it;
}

void _umap_str_it_next(unordered_map_str_it_t** it) {
	// Error check
	if (!it | !(*it)) { return; }

	// Find the next valid position in the buffer
	unordered_map_str_it_t* _it = *it;
	uint8_t* ctrl = NULL;
	unordered_map_str_t* _umap_str = _it->_umap_str;
	do {
		// Increment index
		_it->_index++;

		// Reached the end of the array
		if (_it->_index >= _umap_str->_capacity) {
			free(_it);
			(*it) = NULL;
			break;
		}

		// Evaluate control byte
		ctrl = _umap_str_ctrl(_umap_str, _it->_index);
		if (!(*ctrl & _UMAP_STR_EMPTY)) {
			// Index contains data
			_it->key = *_umap_str_node_key(_umap_str, _it->_index);
			_it->data = _umap_str_node_data(_umap_str, _it->_index);
			break;
		}
	} while(1);

	return;
}

void _umap_str_destroy(unordered_map_str_t* umap_str) {
	// Error check
	if (!umap_str) { return; }

	// Deallocate all strings
	for(size_t i = 0; i<umap_str->_capacity; ++i) {
		uint8_t* ctrl = _umap_str_ctrl(umap_str, i);
		if (!(*ctrl & _UMAP_STR_EMPTY)) {
			free(*_umap_str_node_key(umap_str, i));
		}
	}

	// Deallocate buffer
	free(umap_str);
}
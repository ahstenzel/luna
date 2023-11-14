#include "luna/std/vector.h"

size_t _vec_size(size_t element_size, size_t capacity) {
	size_t c = element_size * capacity;
	if (c / capacity != element_size) { return 0; }
	return LUNA_MAX(sizeof(vector_t), offsetof(vector_t, _buffer) + c);
}

vector_t* _vec_factory(size_t element_size, size_t capacity) {
	size_t buffer_size = _vec_size(element_size, capacity);
	if (buffer_size == 0) { return NULL; }
	vector_t* vec = calloc(1, buffer_size);
	if (!vec) { return NULL; }
	vec->_capacity = capacity;
	vec->_element_size = element_size;
	return vec;
}

vector_t* _vec_resize(vector_t* vec, size_t new_capacity) {
	// Calculate new capacity
	if (new_capacity == 0) {
		size_t c = LUNA_NEXT_POW2(vec->_capacity + 1);
		new_capacity = LUNA_MIN(c, VECTOR_MAX_CAPACITY);
	}
	if (new_capacity > VECTOR_MAX_CAPACITY || new_capacity < vec->_length) { return NULL; }

	// Create new vector & copy data to it
	vector_t* new_vec = _vec_factory(vec->_element_size, new_capacity);
	if (!new_vec) { return NULL; }
	size_t dest_size = vec->_length * vec->_element_size;
	memcpy_s(new_vec->_buffer, dest_size, vec->_buffer, dest_size);
	new_vec->_length = vec->_length;
	free(vec);
	return new_vec;
}

void* _vec_insert(vector_t** vec, size_t index, void* data) {
	// Error check
	if (!vec || !(*vec)) { return NULL; }
	vector_t* _vec = *vec;
	if (index < 0 || index > (_vec->_length + 1)) { return NULL; }

	// Resize container
	if (_vec->_length >= _vec->_capacity) {
		vector_t* temp = _vec_resize(_vec, 0);
		if (!temp) { return NULL; }
		(*vec) = temp;
		_vec = temp;
	}
	
	// Shift over elements
	if (index < _vec->_length) {
		void* dest = (void*)(_vec_pos(_vec, index + 1));
		void* src = (void*)(_vec_pos(_vec, index));
		if (!dest || !src) { return NULL; }
		size_t move_size = _vec->_element_size * (_vec->_length - index);
		memmove_s(dest, move_size, src, move_size);
	}
	
	// Copy element
	uint8_t* dest = _vec_pos(_vec, index);
	size_t dest_size = _vec->_element_size;
	memcpy_s(dest, dest_size, data, dest_size);
	_vec->_length++;
	return (void*)dest;
}

void _vec_remove(vector_t* vec, size_t index, size_t count) {
	// Error check
	if (!vec) { return; }
	if ((index + count) >= vec->_length) { return; }
	
	// Shift over elements
	if (index < vec->_length) {
		void* dest = (void*)(_vec_pos(vec, index));
		void* src = (void*)(_vec_pos(vec, index + count));
		if (!dest || !src) { return; }
		size_t move_size = vec->_element_size * (vec->_length - index - count);
		memmove_s(dest, move_size, src, move_size);
	}

	// Decrement length
	vec->_length -= count;
}

void _vec_swap(vector_t* vec, size_t a, size_t b) {
	// Error check
	if (!vec) { return; }
	if (a >= vec->_length || b >= vec->_length) { return; }

	// Swap elements with temp buffer
	uint8_t* pos_a = _vec_pos(vec, a);
	uint8_t* pos_b = _vec_pos(vec, b);
	uint8_t* _tmp_buffer = malloc(vec->_element_size);
	if (!_tmp_buffer) { return; }
	memmove_s(_tmp_buffer, vec->_element_size, pos_a, vec->_element_size);
	memmove_s(pos_a, vec->_element_size, pos_b, vec->_element_size);
	memmove_s(pos_b, vec->_element_size, _tmp_buffer, vec->_element_size);
	free(_tmp_buffer);
}

void _vec_sort(vector_t* vec) {
	// Error check
	if (!vec) { return; }
}
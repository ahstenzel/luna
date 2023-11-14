#include "luna/std/stack.h"

size_t _stack_size(size_t element_size, size_t capacity) {
	size_t c = element_size * capacity;
	if (c / capacity != element_size) { return 0; }
	return LUNA_MAX(sizeof(stack_t), offsetof(stack_t, _buffer) + c);
}

stack_t* _stack_factory(size_t element_size, size_t capacity) {
	size_t buffer_size = _stack_size(element_size, capacity); 
	if (buffer_size == 0) { return NULL; }
	stack_t* stk = calloc(1, buffer_size);
	if (!stk) { return NULL; }
	stk->_capacity = capacity;
	stk->_element_size = element_size;
	return stk;
}

stack_t* _stack_resize(stack_t* stk, size_t new_capacity) {
	// Calculate new capacity
	if (new_capacity == 0) {
		size_t c = LUNA_NEXT_POW2(stk->_capacity + 1);
		new_capacity = LUNA_MIN(c, STACK_MAX_CAPACITY);
	}
	if (new_capacity > STACK_MAX_CAPACITY || new_capacity < stk->_length) { return NULL; }

	// Create new stack & copy data to it
	stack_t* new_stk = _stack_factory(stk->_element_size, new_capacity);
	if (!new_stk) { return NULL; }
	size_t dest_size = stk->_length * stk->_element_size;
	memcpy_s(new_stk->_buffer, dest_size, stk->_buffer, dest_size);
	new_stk->_length = stk->_length;
	free(stk);
	return new_stk;
}

void* _stack_insert(stack_t** stk, void* data) {
	// Error check
	if (!stk || !(*stk) || !data) { return NULL; }
	stack_t* _stk = *stk;

	// Resize container
	if (_stk->_length >= _stk->_capacity) {
		stack_t* temp = _stack_resize(_stk, 0);
		if (!temp) { return NULL; }
		(*stk) = temp;
		_stk = temp;
	}

	// Copy element
	void* dest = (void*)(_stack_pos(_stk, _stk->_length));
	size_t dest_size = _stk->_element_size;
	memcpy_s(dest, dest_size, data, dest_size);
	_stk->_length++;
	return dest;
}

void _stack_remove(stack_t* stk, size_t count) {
	// Error check
	if (!stk || stk->_length < count) { return; }

	// Decrement _length
	stk->_length -= count;
}
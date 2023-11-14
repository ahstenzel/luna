#include "luna/std/deque.h"

size_t _deque_size(size_t element_size, size_t capacity) {
	size_t c = element_size * capacity;
	if (c / capacity != element_size) { return 0; }
	return LUNA_MAX(sizeof(deque_t), offsetof(deque_t, _buffer) + c);
}

deque_t* _deque_factory(size_t element_size, size_t capacity) {
	size_t buffer_size = _deque_size(element_size, capacity);
	if (buffer_size == 0) { return NULL; }
	deque_t* qu = calloc(1, buffer_size);
	if (!qu) { return NULL; }
	qu->_capacity = capacity;
	qu->_element_size = element_size;
	return qu;
}

deque_t* _deque_resize(deque_t* qu, size_t new_capacity) {
	// Calculate new capacity
	if (new_capacity == 0) {
		size_t c = LUNA_NEXT_POW2(qu->_capacity + 1);
		new_capacity = LUNA_MIN(c, DEQUE_MAX_CAPACITY);
	}
	if (new_capacity > DEQUE_MAX_CAPACITY || new_capacity < qu->_length) { return NULL; }

	// Create new deque & copy data to it
	deque_t* new_qu = _deque_factory(qu->_element_size, new_capacity);
	if (!new_qu) { return NULL; }
	if (qu->_tail <= qu->_head) {
		// Queue wraps around circular buffer, copy in two parts
		size_t _qu_end = (qu->_capacity - qu->_head);
		size_t dest_size_start = _qu_end * qu->_element_size;
		size_t dest_size_end = (qu->_length - _qu_end) * qu->_element_size;
		memcpy_s(_deque_pos(new_qu, 0), dest_size_start, _deque_pos(qu, qu->_head), dest_size_start);
		memcpy_s(_deque_pos(new_qu, _qu_end), dest_size_end, _deque_pos(qu, 0), dest_size_end);
	}
	else {
		// Queue is contained in circular buffer, one copy will get everything
		size_t dest_size = qu->_length * qu->_element_size;
		memcpy_s(new_qu->_buffer, dest_size, _deque_pos(qu, qu->_head), dest_size);
	}
	new_qu->_head = 0;
	new_qu->_tail = qu->_length;
	new_qu->_length = qu->_length;
	free(qu);
	return new_qu;
}

void* _deque_insert_front(deque_t** qu, void* data) {
	// Error check
	if (!qu || !(*qu)) { return NULL; }
	deque_t* _qu = *qu;

	// Resize container
	if (_qu->_length >= _qu->_capacity) {
		deque_t* temp = _deque_resize(_qu, 0);
		if (!temp) { return NULL; }
		(*qu) = temp;
		_qu = temp;
	}

	// Append to tail
	void* dest = _deque_pos(_qu, _qu->_tail);
	size_t dest_size = _qu->_element_size;
	memcpy_s(dest, dest_size, data, dest_size);
	_qu->_tail = (_qu->_tail + 1) % _qu->_capacity;
	_qu->_length++;
	return dest;
}

void* _deque_insert_back(deque_t** qu, void* data) {
	// Error check
	if (!qu || !(*qu)) { return NULL; }
	deque_t* _qu = *qu;

	// Resize container
	if (_qu->_length >= _qu->_capacity) {
		deque_t* temp = _deque_resize(_qu, 0);
		if (!temp) { return NULL; }
		(*qu) = temp;
		_qu = temp;
	}

	// Append to head
	_qu->_length++;
	_qu->_head = (_qu->_head == 0) ? _qu->_capacity : _qu->_head - 1;
	void* dest = _deque_pos(_qu, _qu->_head);
	size_t dest_size = _qu->_element_size;
	memcpy_s(dest, dest_size, data, dest_size);
	return dest;
}

void _deque_remove_front(deque_t* qu, size_t count) {
	// Error check
	if (!qu || qu->_length < count) { return; }

	// Decrement tail
	size_t new_tail = (qu->_tail < count) ? (qu->_capacity - count + qu->_tail) : qu->_tail - count;
	qu->_tail = (qu->_head <= qu->_tail && new_tail < qu->_head) ? qu->_head : new_tail;
	qu->_length -= count;
	return;
}

void _deque_remove_back(deque_t* qu, size_t count) {
	// Error check
	if (!qu || qu->_length < count) { return; }

	// Increment head
	size_t new_head = (qu->_head + count) % qu->_capacity;
	qu->_head = (qu->_head <= qu->_tail && new_head > qu->_tail) ? qu->_tail : new_head;
	qu->_length -= count;
	return;
}
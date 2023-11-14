#include "luna/std/queue.h"

size_t _queue_size(size_t element_size, size_t capacity) {
	size_t c = element_size * capacity;
	if (c / capacity != element_size) { return 0; }
	return LUNA_MAX(sizeof(queue_t), offsetof(queue_t, _buffer) + c);
}

queue_t* _queue_factory(size_t element_size, size_t capacity) {
	size_t buffer_size = _queue_size(element_size, capacity);
	if (buffer_size == 0) { return NULL; }
	queue_t* qu = calloc(1, buffer_size);
	if (!qu) { return NULL; }
	qu->_capacity = capacity;
	qu->_element_size = element_size;
	return qu;
}

queue_t* _queue_resize(queue_t* qu, size_t new_capacity) {
	// Calculate new capacity
	if (new_capacity == 0) {
		size_t c = LUNA_NEXT_POW2(qu->_capacity + 1);
		new_capacity = LUNA_MIN(c, QUEUE_MAX_CAPACITY);
	}
	if (new_capacity > QUEUE_MAX_CAPACITY || new_capacity < qu->_length) { return NULL; }

	// Create new queue & copy data to it
	queue_t* new_qu = _queue_factory(qu->_element_size, new_capacity);
	if (!new_qu) { return NULL; }
	if (qu->_tail <= qu->_head) {
		// Queue wraps around circular buffer, copy in two parts
		size_t _qu_end = (qu->_capacity - qu->_head);
		size_t dest_size_start = _qu_end * qu->_element_size;
		size_t dest_size_end = (qu->_length - _qu_end) * qu->_element_size;
		memcpy_s(_queue_pos(new_qu, 0), dest_size_start, _queue_pos(qu, qu->_head), dest_size_start);
		memcpy_s(_queue_pos(new_qu, _qu_end), dest_size_end, _queue_pos(qu, 0), dest_size_end);
	}
	else {
		// Queue is contained in circular buffer, one copy will get everything
		size_t dest_size = qu->_length * qu->_element_size;
		memcpy_s(new_qu->_buffer, dest_size, _queue_pos(qu, qu->_head), dest_size);
	}
	new_qu->_head = 0;
	new_qu->_tail = qu->_length;
	new_qu->_length = qu->_length;
	free(qu);
	return new_qu;
}

void* _queue_insert(queue_t** qu, void* data) {
	// Error check
	if (!qu || !(*qu)) { return NULL; }
	queue_t* _qu = *qu;

	// Resize container
	if (_qu->_length >= _qu->_capacity) {
		queue_t* temp = _queue_resize(_qu, 0);
		if (!temp) { return NULL; }
		(*qu) = temp;
		_qu = temp;
	}

	// Append to tail
	void* dest = _queue_pos(_qu, _qu->_tail);
	size_t dest_size = _qu->_element_size;
	memcpy_s(dest, dest_size, data, dest_size);
	_qu->_tail = (_qu->_tail + 1) % _qu->_capacity;
	_qu->_length++;
	return dest;
}

void _queue_remove(queue_t* qu, size_t count) {
	// Error check
	if (!qu || qu->_length < count) { return; }

	// Increment head
	size_t new_head = (qu->_head + count) % qu->_capacity;
	qu->_head = (qu->_head <= qu->_tail && new_head > qu->_tail) ? qu->_tail : new_head;
	qu->_length -= count;
}
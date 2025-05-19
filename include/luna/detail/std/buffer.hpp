#pragma once

#include <cstdint>
#include <cstdlib>

namespace luna {
namespace detail {

template<std::size_t A>
class AlignedBuffer {
public:
	AlignedBuffer() :
		m_data(nullptr) {
	}
	AlignedBuffer(std::size_t capacity) :
		m_capacity(capacity) {
		m_data = new std::uint8_t[m_capacity];
	}
	AlignedBuffer(std::size_t capacity, std::uint8_t fill) :
		m_capacity(capacity),
		m_size(capacity) {
		m_data = new std::uint8_t[m_capacity];
		memset(m_data, fill, m_capacity);
	}
	AlignedBuffer(const uint8_t* data, std::size_t len) :
		m_capacity(len),
		m_size(len) {
		m_data = new std::uint8_t[m_capacity];
		memcpy_s(m_data, m_capacity, data, len);
	}
	AlignedBuffer(const AlignedBuffer& other) {
		m_capacity = other.m_capacity;
		m_size = other.m_size;
		m_data = new std::uint8_t[m_capacity];
		memcpy_s(m_data, m_capacity, other.m_data, other.m_capacity);
	}
	AlignedBuffer(AlignedBuffer&& other) noexcept {
		m_capacity = std::move(other.m_capacity);
		m_size = std::move(other.m_size);
		m_data = std::move(other.m_data);
		other.m_data = nullptr;
	}
	~AlignedBuffer() {
		delete[] m_data;
	}

	friend void swap(AlignedBuffer<A>& lhs, AlignedBuffer<A>& rhs) {
		std::swap(lhs.m_capacity, rhs.m_capacity);
		std::swap(lhs.m_size, rhs.m_size);
		std::swap(lhs.m_data, rhs.m_data);
	}

	AlignedBuffer& operator=(AlignedBuffer& other) {
		swap(*this, other);
		return *this;
	}
	AlignedBuffer& operator=(AlignedBuffer&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	std::size_t size() const {
		return m_size;
	}
	bool empty() const {
		return m_size == 0;
	}
	void clear() {
		m_size = 0u;
	}
	std::uint8_t* data(std::size_t offset = 0) const {
		return m_data + offset;
	}

	void push_uint8(std::uint8_t value) {
		push_internal<std::uint8_t>(value);
	}
	void push_uint16(std::uint16_t value) {
		push_internal<std::uint16_t>(value);
	}
	void push_uint32(std::uint32_t value) {
		push_internal<std::uint32_t>(value);
	}
	void push_uint64(std::uint64_t value) {
		push_internal<std::uint64_t>(value);
	}
	void push_string(const std::string& value, std::size_t maxLen) {
		push_string(value.data(), value.size(), maxLen);
	}
	void push_string(const char* value, std::size_t len, std::size_t maxLen) {
		while (m_size + len >= m_capacity) { resize(); }
		for (std::size_t i = 0; i < maxLen; ++i) {
			if (i < len) { m_data[m_size++] = std::uint8_t(value[i]); }
			else { m_data[m_size++] = 0; }
		}
	}

	void pop_uint8() {
		pop_internal<std::uint8_t>();
	}
	void pop_uint16() {
		pop_internal<std::uint16_t>();
	}
	void pop_uint32() {
		pop_internal<std::uint32_t>();
	}
	void pop_uint64() {
		pop_internal<std::uint64_t>();
	}
	void pop_string(std::size_t len) {
		m_size = std::max((std::size_t)0, m_size - len);
	}

	std::uint8_t get_uint8(std::size_t pos) const {
		return get_internal<std::uint8_t>(pos);
	}
	std::uint16_t get_uint16(std::size_t pos) const {
		return get_internal<std::uint16_t>(pos);
	}
	std::uint32_t get_uint32(std::size_t pos) const {
		return get_internal<std::uint32_t>(pos);
	}
	std::uint64_t get_uint64(std::size_t pos) const {
		return get_internal<std::uint64_t>(pos);
	}
	std::string get_string(std::size_t pos, std::size_t len) const {
		if (pos + len > m_size) { throw std::out_of_range("Buffer out of range"); }
		std::string res((char*)&m_data[pos], len);
		std::size_t end = res.find_last_not_of((char)0);
		if (end != std::string::npos) {
			res.erase(res.begin() + end + 1, res.end());
		}
		return res;
	}

	void set_uint8(std::uint8_t value, std::size_t pos) {
		set_internal<std::uint8_t>(value, pos);
	}
	void set_uint16(std::uint16_t value, std::size_t pos) {
		set_internal<std::uint16_t>(value, pos);
	}
	void set_uint32(std::uint32_t value, std::size_t pos) {
		set_internal<std::uint32_t>(value, pos);
	}
	void set_uint64(std::uint64_t value, std::size_t pos) {
		set_internal<std::uint64_t>(value, pos);
	}
	void set_string(const std::string& value, std::size_t pos, std::size_t maxLen) {
		set_string(value.data(), value.size(), pos, maxLen);
	}
	void set_string(const char* value, std::size_t len, std::size_t pos, std::size_t maxLen) {
		if (pos + maxLen > m_size) { throw std::out_of_range("Buffer out of range"); }
		for (std::size_t i = 0; i < maxLen; ++i) {
			if (i < len) { m_data[pos + i] = std::uint8_t(value[i]); }
			else { m_data[pos + i] = 0; }
		}
	}

	AlignedBuffer<A> get_chunk(std::size_t pos, std::size_t len) const {
		if (pos + len > m_size) { throw std::out_of_range("Buffer out of range"); }
		return AlignedBuffer<A>(&m_data[pos], len);
	}
	void pad(std::size_t alignTo) {
		// PKCS#7 padding
		if (m_size % alignTo != 0) {
			std::size_t diff = alignTo - (m_size % alignTo);
			std::size_t byte = std::min(0xFFULL, diff);
			for (std::size_t i = 0; i < diff; ++i) {
				push_uint8((std::uint8_t)byte);
			}
		}
	}

private:
	void resize(std::size_t newCapacity = 0) {
		if (newCapacity == 0) { newCapacity = NextPow2(m_capacity + 1); }
		std::uint8_t* newData = new std::uint8_t[newCapacity];
		memcpy_s(newData, newCapacity, m_data, m_capacity);
		delete[] m_data;
		m_capacity = newCapacity;
	}

	template<typename T>
	void push_internal(T value) {
		while (m_size + sizeof(T) >= m_capacity) { resize(); }
		memcpy_s(&m_data[m_size], m_capacity - m_size, &value, sizeof(T));
		m_size += sizeof(T);
	}

	template<typename T>
	void pop_internal() {
		m_size = std::max((std::size_t)0, m_size - sizeof(T));
	}

	template<typename T>
	T get_internal(std::size_t pos) const {
		if (pos + sizeof(T) > m_size) { throw std::out_of_range(""); }
		T value = T();
		memcpy_s(&value, sizeof(T), &m_data[pos], sizeof(T));
		return value;
	}

	template<typename T>
	void set_internal(T value, std::size_t pos) {
		if (pos + sizeof(T) > m_size) { throw std::out_of_range(""); }
		memcpy_s(&m_data[pos], sizeof(T), &value, sizeof(T));
	}

	alignas(A) std::uint8_t* m_data = nullptr;
	std::size_t m_capacity = 0;
	std::size_t m_size = 0;
};

} // namespace detail

using Buffer = detail::AlignedBuffer<8>;
using Buffer16 = detail::AlignedBuffer<16>;
using Buffer32 = detail::AlignedBuffer<32>;
using Buffer64 = detail::AlignedBuffer<64>;

} // luna
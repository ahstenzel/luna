#pragma once

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <vector>
#include "itsort.hpp"

namespace luna {

/// <summary>
/// Container of sorted values. Uses vector as its underlying container.
/// </summary>
/// <typeparam name="T">Object type</typeparam>
/// <typeparam name="C">Comparator</typeparam>
/// <typeparam name="A">Allocator</typeparam>
template<typename T, typename C = std::less<T>, typename A = std::allocator<T>>
class sorted_list {
public:
	using allocator_type = A;
	using value_type = T;
	using value_compare = C;
	using size_type = std::size_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using difference_type = std::ptrdiff_t;

	using iterator = std::vector::iterator;
	using const_iterator = std::vector::const_iterator;
	using riterator = std::reverse_iterator<iterator>;
	using const_riterator = std::reverse_iterator<const_iterator>;
	using difference_type = std::ptrdiff_t;

private:
	std::vector<T, A> _container;
	value_compare _comp;

	void full_sort() { intro_sort(_container.begin(), _container.end(), _comp); }

	void simple_sort() { insertion_sort(_container.begin(), _container.end(), _comp); }

public:
	// Default constructor
	sorted_list() {}

	// Copy constructor
	sorted_list(const sorted_list& copy) :
		_container(copy.size()) {
		_container = copy._container;
		_comp = copy._comp;
	}

	// Move constructor
	sorted_list(sorted_list&& move) {
		move.swap(*this);
	}

	// Iterator constructor
	template<typename I>
	sorted_list(I begin, I end) :
		_container(std::distance(begin, end)) {
		for (auto it = begin; it != end; ++it) { _container.push_back(*it); }
		full_sort();
	}

	// Initialzer list constructor
	sorted_list(std::initializer_list<value_type> const& list) :
		sorted_list(std::begin(list), std::end(list)) {}

	// Assignment
	sorted_list& operator=(const sorted_list& copy) {
		_container = copy._container;
		_comp = copy._comp;
		return *this;
	}
	sorted_list& operator=(sorted_list&& move) noexcept {
		move.swap(*this);
		return *this;
	}

	// Comparison
	bool operator!=(const sorted_list& rhs) const { return !(*this == rhs); }
	bool operator==(const sorted_list& rhs) const { return (_container == rhs._container && _comp == rhs._comp); }

	// Iterators
	iterator begin() { return _container.begin(); }
	const_iterator begin() const { return _container.begin(); }
	riterator rbegin() { return _container.rbegin(); }
	const_riterator rbegin() const { return _container.rbegin(); }
	const_iterator cbegin() const { return _container.cbegin(); }
	const_iterator crbegin() const { return _container.crbegin(); }
	iterator end() { return _container.end(); }
	const_iterator end() const { return _container.end(); }
	riterator rend() { return _container.rend(); }
	const_riterator rend() const { return _container.rend(); }
	const_iterator cend() const { return _container.cend(); }
	const_iterator crend() const { return _container.crend(); }

	/// <summary>
	/// Get the element at the given index. Performs bounds checking.
	/// </summary>
	/// <param name="index">Array index</param>
	/// <returns>Element reference</returns>
	reference at(size_type index) { return _container.at(index); }

	/// <summary>
	/// Get the element at the given index. Performs bounds checking.
	/// </summary>
	/// <param name="index">Array index</param>
	/// <returns>Const element reference</returns>
	const_reference at(size_type index) const { return _container.at(index); }

	/// <summary>
	/// Get the element at the given index.
	/// </summary>
	/// <param name="index">Array index</param>
	/// <returns>Element reference</returns>
	reference operator[](size_type index) { return _container[index]; }

	/// <summary>
	/// Get the element at the given index.
	/// </summary>
	/// <param name="index">Array index</param>
	/// <returns>Const element reference</returns>
	const_reference operator[](size_type index) const { return _container[index]; }

	/// <summary>
	/// Get the first element in the container.
	/// </summary>
	/// <returns>Element reference</returns>
	reference front() { return _container.front(); }

	/// <summary>
	/// Get the first element in the container.
	/// </summary>
	/// <returns>Const element reference</returns>
	const_reference front() const { return _container.front(); }
	
	/// <summary>
	/// Get the last element in the container.
	/// </summary>
	/// <returns>Element reference</returns>
	reference back() { return _container.back(); }
	
	/// <summary>
	/// Get the last element in the container.
	/// </summary>
	/// <returns>Const element reference</returns>
	const_reference back() const { return _container.back(); }

	/// <summary>
	/// Get the number of elements in the container.
	/// </summary>
	std::size_t size() const { return _container.size(); }

	/// <summary>
	/// Check if the container is empty.
	/// </summary>
	bool empty() const { return _container.empty(); }

	/// <summary>
	/// Remove all elements from the container.
	/// </summary>
	void clear() { _container.clear(); }

	/// <summary>
	/// Swap the contents of this container with another.
	/// </summary>
	void swap(sorted_list& other) noexcept { 
		std::swap(_container, other._container); 
		std::swap(_comp, other._comp);
	}

	/// <summary>
	/// Pre-allocate memory in this container.
	/// </summary>
	void reserve(size_type new_capacity) { _container.reserve(new_capacity); }

	/// <summary>
	/// Add a new element to the list in its sorted position.
	/// </summary>
	void push(value_type const& element) {
		_container.push_back(element);
		simple_sort();
	}

	/// <summary>
	/// Create a new element in the list in its sorted position.
	/// </summary>
	template<typename... Args>
	void emplace(Args&&... args) {
		_container.emplace_back(std::move(args)...);
		simple_sort();
	}

	/// <summary>
	/// Remove the first element in the list.
	/// </summary>
	void pop_front() { _container.erase(_container.begin()); }

	/// <summary>
	/// Remove the last element in the list.
	/// </summary>
	void pop_back() { _container.pop_back(); }

	/// <summary>
	/// Remove the element at a certain position in the list.
	/// </summary>
	void erase(iterator it) { _container.erase(it); }
};

}
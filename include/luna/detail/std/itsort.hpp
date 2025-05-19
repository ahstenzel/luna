#pragma once

#include <iterator>
#include <algorithm>

namespace luna {
namespace detail {

/// <summary>
/// <para/>Insertion sort
/// <para/>Time: O(n ^ 2)
/// <para/>Space: O(1) (in-place)
/// <para/>Stable
/// <para/>Step through the set one at a time and find where the current value fits in the
/// previous positions, which have already been sorted. Good for small sets, or
/// when a set is mostly sorted already.
/// </summary>
class InsertionSort {
public:
	//Sort using provided compare function
	template<class RandomAccessIterator, class Compare>
	static void sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
		for (auto i = std::next(begin, 1); i != end; ++i) {
			for (auto j = i; j != begin; --j) {
				if (comp(*j, *(j - 1))) { std::swap(*j, *(j - 1)); }
				else { break; }
			}
		}
	}
private:
	InsertionSort() {}
	~InsertionSort() {}
};

/// <summary>
/// <para/>Quick Sort
/// <para/>Time: worst O(n ^ 2), avg O(n log n)
/// <para/>Space: O(1) (in-place)
/// <para/>Not Stable
/// <para/>Divide the set into subsets by some pivots, and recursively sort each subset.
/// Not good for mostly sorted arrays, or ones with many repeated elements. This
/// implementation uses dual pivot partitioning and Insertion Sort for partition
/// sizes less than 32.
/// </summary>
class QuickSort {
public:
	template<class RandomAccessIterator, class Compare>
	static void BidirectionalSafeSort(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		//Define endpoints
		auto right = std::prev(end);
		auto left = begin;
		auto dist = std::distance(left, right);
		if (dist <= 0) { return; }
		//Define pivots
		auto id1 = std::next(left, dist / 3);
		auto id2 = std::prev(right, dist / 3);
		std::swap(*id1, *left);
		std::swap(*id2, *right);
		if (comp(*right, *left)) { std::swap(*left, *right); }
		if (dist == 1) { return; }
		auto p1 = left;
		auto p2 = right;
		//Pivot 1 < Pivot 2
		if (comp(*p1, *p2)) {
			//Define partition
			auto lower = std::next(left);
			auto upper = std::prev(right);
			while (!comp(*upper, *p2)) { upper--; }
			while (!comp(*p1, *lower)) { lower++; }
			//Sort out elements less than the pivot
			auto it = lower;
			while (std::distance(it, upper) >= 0) {
				if (!comp(*p1, *it)) {
					std::swap(*it, *lower);
					lower++;
				}
				else if (!comp(*it, *p2)) {
					std::swap(*it, *upper);
					upper--;
					while (!comp(*upper, *p2)) { upper--; }
					if (!comp(*p1, *it)) {
						std::swap(*it, *lower);
						lower++;
					}
				}
				it++;
			}
			//Sort partitions
			std::swap(*(std::prev(lower)), *left);
			std::swap(*(std::next(upper)), *right);
			BidirectionalSafeSort(begin, std::prev(lower), comp);
			BidirectionalSafeSort(std::next(upper, 2), end, comp);
			BidirectionalSafeSort(lower, std::next(upper), comp);
		}
		//Pivot 1 == Pivot 2
		else {
			//Define partition
			auto lower = std::next(left);
			auto upper = std::prev(right);
			while (comp(*p1, *upper)) { upper--; }
			while (comp(*lower, *p1)) { lower++; }
			//Sort out elements less than the pivot
			auto it = lower;
			while (std::distance(it, upper) >= 0) {
				if (comp(*it, *p1)) {
					std::swap(*it, *lower++);
					lower++;
				}
				else if (comp(*p1, *it)) {
					std::swap(*it, *upper);
					upper--;
					while (!comp(*p1, *upper)) { upper--; }
					if (comp(*it, *p1)) {
						std::swap(*it, *lower);
						lower++;
					}
				}
				it++;
			}
			//Sort partitions
			std::swap(*(std::prev(lower)), *left);
			std::swap(*(std::next(upper)), *right);
			BidirectionalSafeSort(begin, std::prev(lower), comp);
			BidirectionalSafeSort(std::next(upper, 2), end, comp);
		}
	}

	typedef std::ptrdiff_t Index;
	template <class Iterator, class Compare>
	static void sort(Iterator a, Iterator finish, Compare compare)
	{
		Index right = (finish - a) - 1;
		Index left = 0;
		if (right <= left) { return; }
		auto idx1 = left + (right - left) / 3;
		auto idx2 = right - (right - left) / 3;
		std::swap(a[idx1], a[left]);
		std::swap(a[idx2], a[right]);
		if (compare(a[right], a[left])) { std::swap(a[left], a[right]); }
		if (right - left == 1) { return; }
		auto pivot1 = a[left];
		auto pivot2 = a[right];
		if (compare(pivot1, pivot2)) {
			// pivot1 < pivot2
			Index less = left + 1;
			Index greater = right - 1;
			while (!compare(a[greater], pivot2)) { greater--; }
			while (!compare(pivot1, a[less])) { less++; }
			Index k = less;
			while (k <= greater) {
				if (!compare(pivot1, a[k])) {
					std::swap(a[k], a[less++]);
				}
				else if (!compare(a[k], pivot2)) {
					std::swap(a[k], a[greater--]);
					while (!compare(a[greater], pivot2)) { greater--; }
					if (!compare(pivot1, a[k])) {
						std::swap(a[k], a[less++]);
					}
				}
				k++;
			}
			std::swap(a[less - 1], a[left]);
			std::swap(a[greater + 1], a[right]);
			// switch to insertion if partition size is too small
			if ((finish - a) <= 32) {
				InsertionSort::sort(a, finish, compare);
			}
			else {
				sort(a, a + less - 1, compare);
				sort(a + greater + 2, finish, compare);
				sort(a + less, a + greater + 1, compare);
			}
		}
		else
		{
			// pivot1 == pivot2
			Index less = left + 1;
			Index greater = right - 1;
			while (compare(pivot1, a[greater])) { greater--; }
			while (compare(a[less], pivot1)) { less++; }
			Index k = less;
			while (k <= greater) {
				if (compare(a[k], pivot1)) {
					std::swap(a[k], a[less++]);
				}
				else if (compare(pivot1, a[k])) {
					std::swap(a[k], a[greater--]);
					while (compare(pivot1, a[greater])) { greater--; }
					if (compare(a[k], pivot1)) {
						std::swap(a[k], a[less++]);
					}
				}
				k++;
			}
			std::swap(a[less - 1], a[left]);
			std::swap(a[greater + 1], a[right]);
			// switch to insertion if partition size is too small
			if ((finish - a) <= 32) {
				InsertionSort::sort(a, finish, compare);
			}
			else {
				sort(a, a + less - 1, compare);
				sort(a + greater + 2, finish, compare);
			}
		}
	}
private:
	QuickSort() {}
	~QuickSort() {}
};

/// <summary>
/// <para/>Heap Sort
/// <para/>Time: O(n log n)
/// <para/>Space: O(1) (in-place)
/// <para/>Not Stable
/// <para/>Create a heap from the data, and move the largest member of the heap into the
/// sorted list. Quicksort can be faster in some cases, but this is faster on
/// average, and tends to consume less memory.
/// </summary>
class HeapSort {
public:
	template<class RandomAccessIterator, class Compare>
	static void sort(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		//Build the initial heap
		make_heap(begin, end, comp);
		//Extract from the sorted heap
		sort_heap(begin, end, comp);
	}

	template<class RandomAccessIterator, class Compare>
	static void make_heap(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		DifferenceType size = std::distance(begin, end);
		for (DifferenceType i = size / 2; i >= 0; i--) {
			//Heapify tree rooted at node i
			adjust_heap(begin, size, i, comp);
		}
	}

	template<class RandomAccessIterator, class Compare>
	static void adjust_heap(RandomAccessIterator begin, __int64 size, __int64 heapPos, Compare comp) {
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		while (heapPos < size) {
			DifferenceType childPos = (heapPos * 2) + 1;
			if (childPos < size) {
				auto child = std::next(begin, childPos);
				auto heap = std::next(begin, heapPos);
				if ((childPos + 1 < size) && (comp(*child, *(std::next(child))))) {
					child = std::next(begin, ++childPos);
				}
				if (comp(*child, *heap)) { return; }
				else { std::swap(*heap, *child); }
			}
			heapPos = childPos;
		}
	}

	template<class RandomAccessIterator, class Compare>
	static void sort_heap(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		DifferenceType pos = std::distance(begin, end) - 1;
		while (pos > 0) {
			std::swap(*begin, *(std::next(begin, pos)));
			adjust_heap(begin, pos, 0, comp);
			pos--;
		}
	}

private:
	HeapSort() {}
	~HeapSort() {}
};

/// <summary>
/// <para/>Merge Sort
/// <para/>Time: O(n log n)
/// <para/>Space: O(1) (in-place)
/// <para/>Stable
/// <para/>Divide the list into its smallest possible units, then recursively rearrage
/// each sublist until you've reconstructed the entire thing. About the same
/// time complexity as Heap Sort, but may offer better data caching performance.
/// </summary>
class MergeSort {
public:
	template<class RandomAccessIterator, class Compare>
	static void sort(RandomAccessIterator begin, RandomAccessIterator low, RandomAccessIterator high, Compare comp) {
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		DifferenceType size = std::distance(low, high);
		if (size > 1) {
			auto center = std::next(low, size / 2);
			sort(begin, low, center, comp);
			sort(begin, center, high, comp);
			std::inplace_merge(low, center, high, comp);
		}
	}
private:
	MergeSort() {}
	~MergeSort() {}
};

/// <summary>
/// <para/>Intro Sort
/// <para/>Time: O(n log n)
/// <para/>Space: O(1) (in-place)
/// <para/>Not Stable
/// <para/>Start with Quick Sort, use Heap Sort when recursion depth reaches a certain
/// threshold, and switch to Insertion Sort for small partition sizes, effectively
/// getting the best properties of all three algorithms.
/// </summary>
class IntroSort {
public:
	template<class RandomAccessIterator, class Compare>
	static void sort(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		//K-sort the array using a combination of Quick Sort and Heap Sort
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		DifferenceType size = std::distance(begin, end);
		DifferenceType maxDepth = (DifferenceType)log2(size);
		intro_sort(begin, end, maxDepth, comp);
		//Use Insertion sort to finish the sorting
		InsertionSort::sort(begin, end, comp);
	}

	template<class RandomAccessIterator, class DifferenceType, class Compare>
	static void intro_sort(RandomAccessIterator begin, RandomAccessIterator end, DifferenceType depth, Compare comp) {
		DifferenceType size = std::distance(begin, end);
		//Partition size too small
		if (size <= 32) { return; }
		//Max depth reached
		else if (depth == 0) { HeapSort::sort(begin, end, comp); }
		//Partition & sort
		else {
			//Pick a good pivot & put it at the start
			auto pivot = intro_pivot(begin, end, comp);
			std::swap(*pivot, *begin);
			//Partition based on that pivot & sort the partitions
			auto part = intro_part(begin, end, comp);
			intro_sort(begin, part, depth - 1, comp);
			intro_sort(std::next(part), end, depth - 1, comp);
		}
	}

	template<class RandomAccessIterator, class Compare>
	static RandomAccessIterator intro_part(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		auto left = std::next(begin);
		auto right = std::prev(end);
		//Search range for the crossover point at the median value in the range
		while (true) {
			//March inwards until a mismatch is found on both sides
			while (std::distance(left, right) > 0 && !comp(*right, *begin)) {
				--right;
			}
			while (std::distance(left, right) > 0 && comp(*left, *begin)) {
				++left;
			}
			//Break if the crossover point is found
			if (std::distance(left, right) == 0) { break; }
			//Otherwise, swap the mismatched elements
			else { std::swap(*left, *right); }
		}
		//Exchange the pivot with the crossover
		if (comp(*begin, *left)) { return begin; }
		std::swap(*begin, *left);
		return left;
	}

	template<class RandomAccessIterator, class Compare>
	static RandomAccessIterator intro_pivot(RandomAccessIterator begin, RandomAccessIterator end, Compare comp) {
		//Determine three possible pivots
		typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DifferenceType;
		DifferenceType size = std::distance(begin, end);
		auto m1 = begin;
		auto m2 = std::next(begin, size / 2);
		auto m3 = std::prev(end);
		//Median of 3 pivoting
		if (comp(*m1, *m2)) {
			if (comp(*m2, *m3)) { return m2; }
			else if (comp(*m1, *m3)) { return m3; }
			else { return m1; }
		}
		else {
			if (comp(*m3, *m2)) { return m2; }
			else if (comp(*m3, *m1)) { return m3; }
			else { return m1; }
		}
	}
private:
	IntroSort() {}
	~IntroSort() {}
};
} // detail

template<class RandomAccessIterator>
void insertion_sort(RandomAccessIterator const begin, RandomAccessIterator const end) {
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	detail::InsertionSort::sort(begin, end, std::less<ValueType>());
}
template<class RandomAccessIterator, class Compare>
void insertion_sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
	detail::InsertionSort::sort(begin, end, comp);
}

template<class RandomAccessIterator>
void quick_sort(RandomAccessIterator const begin, RandomAccessIterator const end) {
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	detail::QuickSort::sort(begin, end, std::less<ValueType>());
}
template<class RandomAccessIterator, class Compare>
void quick_sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
	detail::QuickSort::sort(begin, end, comp);
}

template<class RandomAccessIterator>
void heap_sort(RandomAccessIterator const begin, RandomAccessIterator const end) {
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	detail::HeapSort::sort(begin, end, std::less<ValueType>());
}
template<class RandomAccessIterator, class Compare>
void heap_sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
	detail::HeapSort::sort(begin, end, comp);
}

template<class RandomAccessIterator>
void merge_sort(RandomAccessIterator const begin, RandomAccessIterator const end) {
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	detail::MergeSort::sort(begin, begin, end, std::less<ValueType>());
}
template<class RandomAccessIterator, class Compare>
void merge_sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
	detail::MergeSort::sort(begin, begin, end, comp);
}

template<class RandomAccessIterator>
void intro_sort(RandomAccessIterator const begin, RandomAccessIterator const end) {
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	detail::IntroSort::sort(begin, end, std::less<ValueType>());
}
template<class RandomAccessIterator, class Compare>
void intro_sort(RandomAccessIterator const begin, RandomAccessIterator const end, Compare comp) {
	detail::IntroSort::sort(begin, end, comp);
}
} // luna
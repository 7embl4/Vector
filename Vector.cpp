#include "Vector.h"

template <typename T, typename Alloc>
template <typename Iter>
class Vector<T, Alloc>::VectorIterator {
public:
	friend class Vector;

	typedef Iter iterator_type;
	typedef std::random_access_iterator_tag iterator_category;
	typedef ptrdiff_t difference_type;
	typedef iterator_type value_type;
	typedef iterator_type& reference;
	typedef iterator_type* pointer;

	iterator_type* ptr;

private:
	VectorIterator(iterator_type* p) : ptr(p) {}

public:
	VectorIterator(const VectorIterator& other) : ptr(other.ptr) {}

	bool operator== (const VectorIterator& other) {
		return ptr == other.ptr;
	}
	bool operator!= (const VectorIterator& other) {
		return ptr != other.ptr;
	}

	typename VectorIterator::reference operator* () {
		return *ptr;
	}

	VectorIterator& operator++ () {
		++ptr;
		return *this;
	}
	VectorIterator& operator-- () {
		--ptr;
		return *this;
	}

	bool operator< (const VectorIterator& other) {
		return ptr < other.ptr;
	}
	bool operator> (const VectorIterator& other) {
		return ptr > other.ptr;
	}
	bool operator<= (const VectorIterator& other) {
		return ptr <= other.ptr;
	}
	bool operator>= (const VectorIterator& other) {
		return ptr >= other.ptr;
	}

	VectorIterator& operator+= (int n) {
		ptr += n;
		return *this;
	}
	VectorIterator& operator-= (int n) {
		ptr -= n;
		return *this;
	}

	VectorIterator operator+ (int n) {
		VectorIterator result(ptr);
		result += n;
		return result;
	}
	VectorIterator operator- (int n) {
		VectorIterator result(ptr);
		result -= n;
		return result;
	}

	difference_type operator- (const VectorIterator& other) {
		return ptr - other.ptr;
	}
};

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector()
	: alloc(alloc)
	, begin_(nullptr)
	, end_(nullptr)
	, cap_(nullptr) 
{}

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(size_t n, const T& value) 
	: alloc(alloc)
	, begin_(AllocTraits::allocate(alloc, n))
	, end_(begin_ + n)
	, cap_(begin_ + n)
{
	T* it = begin_;
	try {
		for (; it != end_; ++it) {
			AllocTraits::construct(alloc, it, value);
		}
	}
	catch (...) {
		for (T* del = begin_; del != it; ++del) {
			AllocTraits::destroy(alloc, del);
		}
		AllocTraits::deallocate(alloc, begin_, n);
		throw;
	}
}

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(const Vector<T>& other) {
	if (this->capacity() < other.size()) {
		for (T* del = begin_; del != end_; ++del) {
			AllocTraits::destroy(alloc, del);
		}
		AllocTraits::deallocate(alloc, begin_, this->size());
	}

	T* new_begin = (this->capacity() < other.size()) ? AllocTraits::allocate(alloc, other.size()) : begin_;
	T* it = new_begin;
	for (size_t i = 0; i != other.size(); ++i) {
		AllocTraits::construct(alloc, it, other[i]);
		++it;
	}

	begin_ = new_begin;
	end_ = it;
	cap_ = it;
	return *this;
}

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(Vector<T>&& other)
	: alloc(other.alloc)
	, begin_(other.begin_)
	, end_(other.end_)
	, cap_(other.cap_)
{
	other.begin_ = nullptr;
	other.end_ = nullptr;
	other.cap_ = nullptr;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::reserve(size_t new_cap) {
	if (new_cap <= this->capacity()) {
		return;
	}

	T* new_begin = AllocTraits::allocate(alloc, new_cap);
	T* it = new_begin;
	try {
		for (size_t i = 0; i != this->size(); ++i) {
			AllocTraits::construct(alloc, it, std::move_if_noexcept(*(begin_ + i)));
			++it;
		}
	}
	catch (...) {
		for (T* del = new_begin; del != it; ++del) {
			AllocTraits::destroy(alloc, del);
		}
		AllocTraits::deallocate(alloc, new_begin, new_cap);
		throw;
	}

	for (T* del = begin_; del != end_; ++del) {
		AllocTraits::destroy(alloc, del);
	}
	AllocTraits::deallocate(alloc, begin_, this->capacity());

	begin_ = new_begin;
	end_ = it;
	cap_ = new_begin + new_cap;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::resize(size_t new_size) {
	if (new_size <= this->size()) {
		size_t sz = this->size() - new_size;
		for (size_t i = 0; i != sz; ++i) {
			AllocTraits::destroy(alloc, end_ - 1);
			--end_;
		}
	}
	else if (new_size <= this->capacity()) {
		T* it = end_;
		T* new_end = begin_ + new_size;
		try {
			for (; it != new_end; ++it) {
				AllocTraits::construct(alloc, it, T());
			}
		}
		catch (...) {
			for (T* del = end_; del != it; ++del) {
				AllocTraits::destroy(alloc, del);
			}
			throw;
		}
		end_ = new_end;
	}
	else {
		T* new_begin = AllocTraits::allocate(alloc, new_size);
		T* it = new_begin;
		try {
			size_t index = 0;
			for (; index != this->size(); ++index) {
				AllocTraits::construct(alloc, it, std::move_if_noexcept(*(begin_ + index)));
				++it;
			}
			for (; index != new_size; ++index) {
				AllocTraits::construct(alloc, it, T());
				++it;
			}
		}
		catch (...) {
			for (T* del = new_begin; del != it; ++del) {
				AllocTraits::destroy(alloc, del);
			}
			AllocTraits::deallocate(alloc, new_begin, new_size);
			throw;
		}

		for (T* del = begin_; del != end_; ++del) {
			AllocTraits::destroy(alloc, del);
		}
		AllocTraits::deallocate(alloc, begin_, this->capacity());

		begin_ = new_begin;
		end_ = it;
		cap_ = it;
	}
}

template <typename T, typename Alloc>
template <typename U>
void Vector<T, Alloc>::push_back_private(U&& value) {
	if (end_ == cap_) {
		size_t new_cap = (this->capacity() == 0) ? 1 : 2 * this->capacity();
		T* new_begin = AllocTraits::allocate(alloc, new_cap);
		T* it = new_begin;
		try {
			AllocTraits::construct(alloc, it + size(), std::forward<T>(value));
			for (size_t i = 0; i != this->size(); ++i) {
				AllocTraits::construct(alloc, it, std::move_if_noexcept(*(begin_ + i)));
				++it;
			}
		}
		catch (...) {
			for (T* del = new_begin; del != it; ++del) {
				AllocTraits::destroy(alloc, del);
			}
			AllocTraits::deallocate(alloc, new_begin, new_cap);
			throw;
		}

		for (T* del = begin_; del != end_; ++del) {
			AllocTraits::destroy(alloc, del);
		}
		AllocTraits::deallocate(alloc, begin_, this->size());

		begin_ = new_begin;
		end_ = it + 1;
		cap_ = new_begin + new_cap;
	}
	else {
		AllocTraits::construct(alloc, end_, std::forward<T>(value));
		++end_;
	}
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::push_back(const T& value) {
	push_back_private(value);
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::push_back(T&& value) {
	push_back_private(std::move(value));
}

template <typename T, typename Alloc>
T& Vector<T, Alloc>::operator[] (size_t index) {
	if (index < this->size()) {
		return begin_ + index;
	}
	throw;
}

template <typename T, typename Alloc>
const T& Vector<T, Alloc>::operator[] (size_t index) const {
	if (index < this->size()) {
		return begin_ + index;
	}
	throw;
}

template <typename T, typename Alloc>
size_t Vector<T, Alloc>::size() const noexcept {
	return end_ - begin_;
}

template <typename T, typename Alloc>
size_t Vector<T, Alloc>::capacity() const noexcept {
	return cap_ - begin_;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::clear() noexcept {
	for (T* del = begin_; del != end_; ++del) {
		AllocTraits::destroy(alloc, del);
	}
	end_ = begin_;
}

template <typename T, typename Alloc>
bool Vector<T, Alloc>::empty() const noexcept {
	return begin_ == end_;
}

template <typename T, typename Alloc>
typename Vector<T, Alloc>::iterator Vector<T, Alloc>::begin() noexcept {
	return iterator(begin_);
}

template <typename T, typename Alloc>
typename Vector<T, Alloc>::iterator Vector<T, Alloc>::end() noexcept {
	return iterator(end_);
}

template <typename T, typename Alloc>
typename Vector<T, Alloc>::const_iterator Vector<T, Alloc>::cbegin() const noexcept {
	return const_iterator(begin_);
}

template <typename T, typename Alloc>
typename Vector<T, Alloc>::const_iterator Vector<T, Alloc>::cend() const noexcept {
	return const_iterator(end_);
}

template <typename T, typename Alloc>
Vector<T, Alloc>::~Vector() noexcept {
	for (T* del = begin_; del != end_; ++del) {
		AllocTraits::destroy(alloc, del);
	}
	AllocTraits::deallocate(alloc, begin_, this->capacity());
}

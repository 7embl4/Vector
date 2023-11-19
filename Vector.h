#pragma once
#include <memory>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class Vector {
private:
	using AllocTraits = std::allocator_traits<Alloc>;

	/* [[no_unique_address ]] */ Alloc alloc;
	T* begin_;
	T* end_;
	T* cap_;

	template <typename Iter>
	class VectorIterator;

	template <typename U>
	void push_back_private(U&&);

public:
	typedef VectorIterator<T> iterator;
	typedef VectorIterator<const T> const_iterator;

	Vector();
	Vector(size_t, const T& value = T());

	Vector(const Vector&);
	Vector& operator= (const Vector&);

	Vector(Vector&&) noexcept;
	Vector& operator= (Vector&&) noexcept;

	void reserve(size_t);
	void resize(size_t);

	void push_back(const T&);
	void push_back(T&&);
	void pop_back();

	T& operator[] (size_t);
	const T& operator[] (size_t) const;

	size_t size() const noexcept;
	size_t capacity() const noexcept;

	void clear() noexcept;
	bool empty() const noexcept;

	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;

	~Vector() noexcept;
};

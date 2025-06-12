#pragma once

#include "HashTable.h"

template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class Unordered_Set
{
private:
	using Table = HashTable<Key, Key, Hash, KeyEqual, false>;
	Table _table;

public:
	using key_type = Key;
	using value_type = Key;
	using hasher = Hash;
	using key_equal = KeyEqual;
	using size_type = typename Table::size_type;
	using difference_type = std::ptrdiff_t;

	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;

	using iterator = typename Table::iterator;
	using const_iterator = typename Table::const_iterator;

	Unordered_Set();
	~Unordered_Set();
	explicit Unordered_Set(size_type bucket_count, const hasher& hash = hasher(), const key_equal& equal = key_equal());

	template<typename InputIt>
	Unordered_Set(InputIt first, InputIt last, 
		size_type bucket_count = 16, const hasher& hash = hasher(), const key_equal& equal = key_equal());

	Unordered_Set(std::initializer_list<value_type> init,
		size_type bucket_count = 16, const hasher& hash = hasher(), const key_equal& equal = key_equal());

	Unordered_Set(const Unordered_Set& other);
	Unordered_Set(Unordered_Set&& other) noexcept;

	Unordered_Set& operator=(const Unordered_Set& other);
	Unordered_Set& operator=(Unordered_Set&& other) noexcept;
	Unordered_Set& operator=(std::initializer_list<value_type> ilist);

	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator begin() const noexcept;
	const_iterator end() const noexcept;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;

	bool empty() const noexcept;
	size_type size() const noexcept;

	void clear() noexcept;

	std::pair<iterator, bool> insert(const value_type& value);
	std::pair<iterator, bool> insert(value_type&& value);

	template<typename InputIt>
	void insert(InputIt first, InputIt last);

	void insert(std::initializer_list<value_type> ilist);

	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args);

	size_type erase(const key_type& key);

	void swap(Unordered_Set& other) noexcept;

	size_type count(const key_type& key) const;
	
	iterator find(const key_type& key);
	const_iterator find(const key_type& key) const;

	std::pair<iterator, iterator> equal_range(const key_type& key);
	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const;

	size_type bucket_count() const noexcept;
	size_type bucket_size(size_type index) const;
	size_type bucket(const key_type& key) const;

	float load_factor() const noexcept;
	float max_load_factor() const noexcept;
	void max_load_factor(float ml);

	void rehash(size_type count);
	void reserve(size_type count);

	bool operator==(const Unordered_Set& other) const;
	bool operator!=(const Unordered_Set& other) const;
};

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set() = default;

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::~Unordered_Set() = default;

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set(size_type bucket_count, const hasher& hash, const key_equal& equal)
	: _table(bucket_count, hash, equal)
{
}

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set(std::initializer_list<value_type> init, 
	size_type bucket_count, const hasher& hash, const key_equal& equal)
	: _table(bucket_count, hash, equal)
{
	for (const auto& v : init)
		_table.insert(v);
}

template<typename Key, typename Hash, typename KeyEqual>
template<typename InputIt>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set(InputIt first, InputIt last,
	size_type bucket_count, const hasher& hash, const key_equal& equal)
	: _table(bucket_count, hash, equal)
{
	for (; first != last; ++first)
		_table.insert(*first);
}

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set(const Unordered_Set& other)
	: _table(other._table)
{
}

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>::Unordered_Set(Unordered_Set&& other) noexcept
	: _table(std::move(other._table))
{
}

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>& Unordered_Set<Key, Hash, KeyEqual>::operator=(const Unordered_Set& other)
{
	_table = other._table;
	return *this;
}

template<typename Key, typename Hash, typename KeyEqual>
inline Unordered_Set<Key, Hash, KeyEqual>& Unordered_Set<Key, Hash, KeyEqual>::operator=(Unordered_Set&& other) noexcept
{
	_table = std::move(other._table);
	return *this;
}

template<typename Key, typename Hash, typename KeyEqual>
Unordered_Set<Key, Hash, KeyEqual>& Unordered_Set<Key, Hash, KeyEqual>::operator=(std::initializer_list<value_type> ilist)
{
	_table.clear();
	for (const auto& val : ilist)
		_table.insert(val);
	return *this;
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::iterator Unordered_Set<Key, Hash, KeyEqual>::begin() noexcept
{
	return _table.begin();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::iterator Unordered_Set<Key, Hash, KeyEqual>::end() noexcept
{
	return _table.end();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator Unordered_Set<Key, Hash, KeyEqual>::begin() const noexcept
{
	return _table.begin();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator Unordered_Set<Key, Hash, KeyEqual>::end() const noexcept
{
	return _table.end();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator Unordered_Set<Key, Hash, KeyEqual>::cbegin() const noexcept
{
	return _table.cbegin();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator Unordered_Set<Key, Hash, KeyEqual>::cend() const noexcept
{
	return _table.cend();
}

template<typename Key, typename Hash, typename KeyEqual>
bool Unordered_Set<Key, Hash, KeyEqual>::empty() const noexcept
{
	return _table.empty();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::size() const noexcept
{
	return _table.size();
}

template<typename Key, typename Hash, typename KeyEqual>
void Unordered_Set<Key, Hash, KeyEqual>::clear() noexcept
{
	_table.clear();
}

template<typename Key, typename Hash, typename KeyEqual>
std::pair<typename Unordered_Set<Key, Hash, KeyEqual>::iterator, bool> 
		Unordered_Set<Key, Hash, KeyEqual>::insert(const value_type& value)
{
	return _table.insert(value);
}

template<typename Key, typename Hash, typename KeyEqual>
std::pair<typename Unordered_Set<Key, Hash, KeyEqual>::iterator, bool> Unordered_Set<Key, Hash, KeyEqual>::insert(value_type&& value)
{
	return _table.insert(std::move(value));
}

template<typename Key, typename Hash, typename KeyEqual>
template<typename InputIt>
inline void Unordered_Set<Key, Hash, KeyEqual>::insert(InputIt first, InputIt last)
{
	for (; first != last; ++first)
		_table.insert(*first);
}

template<typename Key, typename Hash, typename KeyEqual>
void Unordered_Set<Key, Hash, KeyEqual>::insert(std::initializer_list<value_type> ilist)
{
	for (const auto& val : ilist)
		_table.insert(val);
}

template<typename Key, typename Hash, typename KeyEqual>
template<typename... Args>
std::pair<typename Unordered_Set<Key, Hash, KeyEqual>::iterator, bool> Unordered_Set<Key, Hash, KeyEqual>::emplace(Args&&... args)
{
	return _table.emplace(std::forward<Args>(args)...);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::erase(const key_type& key)
{
	return _table.erase(key);
}

template<typename Key, typename Hash, typename KeyEqual>
inline void Unordered_Set<Key, Hash, KeyEqual>::swap(Unordered_Set& other) noexcept
{
	_table.swap(other._table);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::count(const key_type& key) const
{
	return _table.count(key);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::iterator Unordered_Set<Key, Hash, KeyEqual>::find(const key_type& key)
{
	return _table.find(key);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator Unordered_Set<Key, Hash, KeyEqual>::find(const key_type& key) const
{
	return _table.find(key);
}

template<typename Key, typename Hash, typename KeyEqual>
std::pair<typename Unordered_Set<Key, Hash, KeyEqual>::iterator, typename Unordered_Set<Key, Hash, KeyEqual>::iterator> 
		Unordered_Set<Key, Hash, KeyEqual>::equal_range(const key_type& key)
{
	return _table.equal_range(key);
}

template<typename Key, typename Hash, typename KeyEqual>
std::pair<typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator, typename Unordered_Set<Key, Hash, KeyEqual>::const_iterator> 
		Unordered_Set<Key, Hash, KeyEqual>::equal_range(const key_type& key) const
{
	return _table.equal_range(key);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::bucket_count() const noexcept
{
	return _table.bucket_count();
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::bucket_size(size_type index) const
{
	return _table.bucket_size(index);
}

template<typename Key, typename Hash, typename KeyEqual>
typename Unordered_Set<Key, Hash, KeyEqual>::size_type Unordered_Set<Key, Hash, KeyEqual>::bucket(const key_type& key) const
{
	return _table.bucket(key);
}

template<typename Key, typename Hash, typename KeyEqual>
float Unordered_Set<Key, Hash, KeyEqual>::load_factor() const noexcept
{
	return _table.load_factor();
}

template<typename Key, typename Hash, typename KeyEqual>
float Unordered_Set<Key, Hash, KeyEqual>::max_load_factor() const noexcept
{
	return _table.max_load_factor();
}

template<typename Key, typename Hash, typename KeyEqual>
void Unordered_Set<Key, Hash, KeyEqual>::max_load_factor(float ml)
{
	_table.max_load_factor(ml);
}

template<typename Key, typename Hash, typename KeyEqual>
void Unordered_Set<Key, Hash, KeyEqual>::rehash(size_type count)
{
	_table.reserve(count);
}

template<typename Key, typename Hash, typename KeyEqual>
void Unordered_Set<Key, Hash, KeyEqual>::reserve(size_type count)
{
	_table.reserve(count);
}

template<typename Key, typename Hash, typename KeyEqual>
inline bool Unordered_Set<Key, Hash, KeyEqual>::operator==(const Unordered_Set& other) const
{
	return _table == other._table;
}

template<typename Key, typename Hash, typename KeyEqual>
inline bool Unordered_Set<Key, Hash, KeyEqual>::operator!=(const Unordered_Set& other) const
{
	return !(*this == other);
}


template<typename Key, typename Hash, typename KeyEqual>
void swap(Unordered_Set<Key, Hash, KeyEqual>& lhs, Unordered_Set<Key, Hash, KeyEqual>& rhs) noexcept
{
	lhs.swap(rhs);
}



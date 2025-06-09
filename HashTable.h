#pragma once

#include <vector>
#include <utility>
#include <functional>
#include <type_traits>
#include <iterator>
#include <cstddef>
#include <stdexcept>
#include "Unordered_Set.h"

template<typename Key, typename T = Key, typename Hash = std::hash<Key>, 
    typename KeyEqual = std::equal_to<Key>, bool AllowDuplicates = false>
class HashTable
{
public:
    using key_type = Key;
    using mapped_type = T;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using size_type = std::size_t;
    using value_type = std::conditional_t<std::is_same_v<Key, T>, Key, std::pair<const Key, T>>;

private:
    struct Node
    {
        Node* _next;
        value_type _data;

        template<typename K, typename V>
        Node(K&& key, V&& value, Node* next = nullptr)
            : _next(next)
            , _data(std::forward<K>(key), std::forward<V>(value)) 
        {
        }

        template<typename K, typename = std::enable_if_t<std::is_constructible_v<value_type, K&&>>>
        explicit Node(K&& key_or_value, Node* next = nullptr)
            : _next(next)
            , _data(std::forward<K>(key_or_value))
        {
        }

        ~Node() = default;
    };

    using Bucket = Node*;

    std::vector<Bucket> _buckets;
    size_type _size = 0;
    float _max_load = 0.75f;
    Hash _hash_fn;
    KeyEqual _key_eq;

    const key_type& get_key(const value_type& val) const;

    const key_type& get_key_from_value(const value_type& val) const;

    void check_load();

    void rehash(size_type new_cap);

    Node* find_node(const key_type& key, size_type index) const;

    size_type bucket_index(const Key& key) const;

public:
    template<bool IsConst>
    class HashIterator
    {
        using NodePtr = std::conditional_t<IsConst, const Node*, Node*>;
        using BucketPtr = std::conditional_t<IsConst, const Bucket*, Bucket*>;

        BucketPtr _bucket;
        BucketPtr _bucket_end;
        NodePtr _node;

        void skip_empty();

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = HashTable::value_type;
        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

        HashIterator(BucketPtr bucket, BucketPtr end, NodePtr node = nullptr);
        ~HashIterator();

        reference operator*() const;
        pointer operator->() const;

        HashIterator& operator++();
        HashIterator operator++(int);

        bool operator==(const HashIterator& rhs) const;
        bool operator!=(const HashIterator& rhs) const;
    };

    using iterator = HashIterator<false>;
    using const_iterator = HashIterator<true>;


    HashTable(size_type capacity = 16);
    HashTable(std::initializer_list<value_type> init);
    HashTable(const HashTable& other);
    HashTable(HashTable&& other) noexcept;
    ~HashTable();

    HashTable& operator=(const HashTable& other);
    HashTable& operator=(HashTable&& other);

    std::pair<iterator, bool> insert(const key_type& key, const mapped_type& value);
    std::pair<iterator, bool> insert(const value_type& kv);
    std::pair<iterator, bool> insert(value_type&& kv);

    std::pair<iterator, bool> insert_or_assign(const key_type& key, mapped_type&& val);
    
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args);

    template<typename... Args>
    std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args);

    iterator find(const key_type& key);
    const_iterator find(const key_type& key) const;

    size_type erase(const key_type& key);

    mapped_type& operator[](const Key& key);
    mapped_type& operator[](Key&& key);

    mapped_type& at(const Key& key);
    const mapped_type& at(const Key& key) const;

    std::pair<iterator, iterator> equal_range(const Key& key);
    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const;

    void clear();

    size_type size() const;
    bool empty() const;

    size_type count(const Key& key) const;

    size_type bucket_count() const;
    size_type bucket_size(size_type index) const;
    size_type bucket(const Key& key) const;

    float load_factor() const;
    float max_load_factor() const;

    void max_load_factor(float new_max);

    void reserve(size_type n);

    void swap(HashTable& other) noexcept;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    bool operator==(const HashTable& other) const;
    bool operator!=(const HashTable& other) const;

    template<typename K, typename M, typename H, typename E, bool D>
    friend void swap(HashTable<K, M, H, E, D>& lhs, HashTable<K, M, H, E, D>& rhs)  noexcept;
};

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline const typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::key_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::get_key(const value_type& val) const
{
    if constexpr (std::is_same_v<key_type, mapped_type>)
        return val;
    else
        return val.first;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline const typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::key_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::get_key_from_value(const value_type& val) const
{
    if constexpr (std::is_same_v<key_type, mapped_type>)
        return val;
    else
        return val.first;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::check_load()
{
    if (static_cast<float>(_size) / _buckets.size() > _max_load)
        rehash(_buckets.size() * 2);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::rehash(size_type new_cap)
{
    std::vector<Bucket> new_buckets(new_cap, nullptr);
    for (auto& bucket : _buckets)
    {
        Node* node = bucket;
        while (node)
        {
            Node* next = node->_next;
            size_type index = _hash_fn(get_key(node->_data)) % new_cap;
            node->_next = new_buckets[index];
            new_buckets[index] = node;
            node = next;
        }
    }
    _buckets.swap(new_buckets);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::Node* 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::find_node(const key_type& key, size_type index) const
{
    Node* current = _buckets[index];    
    while (current)
    {
        if (_key_eq(get_key(current->_data), key))
            return current;
        current = current->_next;
    }
    return nullptr; 
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::bucket_index(const Key& key) const
{
    return _hash_fn(key) % _buckets.size();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
inline void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::skip_empty()
{
    while (_bucket != _bucket_end && !*_bucket)
        ++_bucket;

    if (_bucket != _bucket_end)
        _node = *_bucket;
    else
        _node = nullptr;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::HashIterator(BucketPtr bucket, BucketPtr end, NodePtr node)
    : _bucket(bucket)
    , _bucket_end(end)
    , _node(node)
{
    if (!_node)
        skip_empty();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::~HashIterator()
{
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::reference 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator*() const
{
    return _node->_data;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::pointer 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator->() const
{
    return &_node->_data;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator++()
{
    _node = _node->_next;
    if (!_node)
    {
        ++_bucket;
        skip_empty();
    }
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst> 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator++(int)
{
    HashIterator temp = *this;
    ++(*this);
    return temp;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
inline bool HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator==(const HashIterator& rhs) const
{
    return _node == rhs._node;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<bool IsConst>
inline bool HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashIterator<IsConst>::operator!=(const HashIterator& rhs) const
{
    return !(*this == rhs);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashTable(size_type capacity)
    : _buckets(capacity, nullptr)
{
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashTable(std::initializer_list<value_type> init)
    : HashTable(init.size())
{
    for (const auto& kv : init)
        insert(kv);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashTable(const HashTable& other)
{
    reserve(other._buckets.size());
    for (const auto& kv : other)
        insert(kv);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::HashTable(HashTable&& other) noexcept
{
    swap(other);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::~HashTable()
{
    clear();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator=(const HashTable& other)
{
    if (this != &other)
    {
        clear();
        reserve(other._buckets.size());
        for (const auto& kv : other)
            insert(kv);
    }
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator=(HashTable&& other)
{
    if (this != &other)
        swap(other);
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool>
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::insert(const key_type& key, const mapped_type& value) 
{
    return insert(value_type(key, value));
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool>
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::insert(const value_type& kv) 
{
    const key_type& key = get_key_from_value(kv);
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    if constexpr (!AllowDuplicates) 
    {
        while (node) 
        {
            if (_key_eq(get_key(node->_data), key)) 
                return { iterator(&_buckets[index], &_buckets[0] + _buckets.size(), node), false };
            node = node->_next;
        }
    }

    _buckets[index] = new Node(kv, _buckets[index]);
    ++_size;
    check_load();
    return { iterator(&_buckets[index], &_buckets[0] + _buckets.size(), _buckets[index]), true };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool>
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::insert(value_type&& kv) 
{
    const key_type& key = get_key_from_value(kv);
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    if constexpr (!AllowDuplicates) 
    {
        while (node) 
        {
            if (_key_eq(get_key(node->_data), key)) 
                return { iterator(&_buckets[index], &_buckets[0] + _buckets.size(), node), false };
            node = node->_next;
        }
    }

    _buckets[index] = new Node(std::move(kv), _buckets[index]);
    ++_size;
    check_load();
    return { iterator(&_buckets[index], &_buckets[0] + _buckets.size(), _buckets[index]), true };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool> 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::insert_or_assign(const key_type& key, mapped_type&& val)
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    while (node)
    {
        if (_key_eq(get_key(node->_data), key))
        {
            node->_data.second = std::move(val);
            return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), node), false };
        }
        node = node->_next;
    }

    value_type new_value(key, std::move(val));
    _buckets[index] = new Node(std::move(new_value), _buckets[index]);
    ++_size;
    check_load();
    return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), _buckets[index]), true };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<typename ...Args>
inline std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool> 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::emplace(Args && ...args)
{
    value_type val(std::forward<Args>(args)...);
    return insert(val);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
template<typename ...Args>
inline std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, bool> 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::try_emplace(const Key& key, Args && ...args)
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    if constexpr (!AllowDuplicates)
    {
        while (node)
        {
            if (_key_eq(get_key(node->_data), key))
                return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), node), false };
            node = node->_next;
        }
    }

    value_type val(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<Args>(args)...));
    _buckets[index] = new Node(std::move(val), _buckets[index]);
    ++_size;
    check_load();
    return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), _buckets[index]), true };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::find(const key_type& key)
{
    size_type index = _hash_fn(key) % _buckets.size();
    Node* node = find_node(key, index);
    if (!node)
        return end();
    return iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), node);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::find(const key_type& key) const
{
    size_type index = _hash_fn(key) % _buckets.size();
    Node* node = find_node(key, index);
    if (!node)
        return end();
    return const_iterator(_buckets.data() + index, _buckets.data() + _buckets.size(), node);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::erase(const key_type& key)
{
    size_type index = _hash_fn(key) % _buckets.size();
    Node*& head = _buckets[index];
    Node* current = head;
    Node* prev = nullptr;
    size_type count = 0;

    while (current)
    {
        if (_key_eq(get_key(current->_data), key))
        {
            Node* to_delete = current;
            if (prev)
                prev->_next = current->_next;
            else
                head = current->_next;

            current = current->_next;
            delete to_delete;
            --_size;
            ++count;
            if constexpr (!AllowDuplicates)
                break;
        }
        else
        {
            prev = current;
            current = current->_next;
        }
    }
    return count;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::mapped_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator[](const Key& key)
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    while (node)
    {
        if (_key_eq(get_key(node->_data), key))
            return node->_data.second;
        node = node->_next;
    }

    value_type new_pair(key, mapped_type());
    _buckets[index] = new Node(std::move(new_pair), _buckets[index]);
    ++_size;
    check_load();
    return _buckets[index]->_data.second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::mapped_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator[](Key&& key)
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];

    while (node)
    {
        if (_key_eq(get_key(node->_data), key))
            return node->_data.second;
    }

    value_type new_pair(std::move(key), mapped_type());
    _buckets[index] = new Node(std::move(new_pair), _buckets[index]);
    ++_size;
    check_load();
    return _buckets[index]->_data.second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::mapped_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::at(const Key& key)
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];
    if (!node)
        throw std::out_of_range("HashTable::at - key not found");
    return node->_data.second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline const typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::mapped_type& 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::at(const Key& key) const
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];
    if (!node)
        throw std::out_of_range("HashTable::at - key not found");
    return node->_data.second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator, 
    typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator> 
    HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::equal_range(const Key& key)
{
    auto it = find(key);
    if (it == end())
        return { it, it };
    auto next = it;
    ++next;
    return { it, next };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline std::pair<typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator, 
    typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator> 
    HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::equal_range(const Key& key) const
{
    auto it = find(key);
    if (it == end())
        return { it, it };
    auto next = it;
    ++next;
    return { it, next };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::clear()
{
    for (auto& bucket : _buckets)
    {
        Node* current = bucket;
        while (current)
        {
            Node* next = current->_next;
            delete current;
            current = next;
        }
        bucket = nullptr;
    }
    _size = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::count(const Key& key) const
{
    size_type index = bucket_index(key);
    Node* node = _buckets[index];
    size_type cnt = 0;
    
    while (node)
    {
        if (_key_eq(get_key(node->_data), key))
            ++cnt;
        node = node->_next;
    }
    return cnt;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size() const
{
    return _size;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
bool HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::empty() const
{
    return _size == 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::bucket_count() const
{
    return _buckets.size();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::bucket_size(size_type index) const
{
    size_type count = 0;
    Node* node = _buckets[index];

    while (node)
    {
        ++count;
        node = node->_next;
    }
    return count;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::size_type 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::bucket(const Key& key) const
{
    return bucket_index(key);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
float HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::load_factor() const
{
    return _buckets.empty() ? 0.0f : static_cast<float>(_size) / _buckets.size();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
float HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::max_load_factor() const
{
    return _max_load;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::max_load_factor(float new_max)
{
    _max_load = new_max;
    check_load();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::reserve(size_type n)
{
    size_type new_cap = std::max(n, _size);
    new_cap = std::max<size_type>(new_cap, 8);
    rehash(new_cap);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline void HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::swap(HashTable& other) noexcept
{
    std::swap(_buckets, other._buckets);
    std::swap(_size, other._size);
    std::swap(_max_load, other._max_load);
    std::swap(_hash_fn, other._hash_fn);
    std::swap(_key_eq, other._key_eq);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::begin()
{
    return iterator(_buckets.data(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::iterator HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::end()
{
    return iterator(_buckets.data() + _buckets.size(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::begin() const
{
    return const_iterator(_buckets.data(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::end() const
{
    return const_iterator(_buckets.data() + _buckets.size(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::cbegin() const
{
    return begin();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
typename HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::const_iterator 
            HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::cend() const
{
    return end();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline bool HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator==(const HashTable& other) const
{
    if (_size != other._size)
        return false;

    for (const auto& kv : *this)
    {
        auto it = other.find(get_key(kv));
        if (it == other.end() || !(kv == *it))
            return false;
    }

    return true;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, bool AllowDuplicates>
inline bool HashTable<Key, T, Hash, KeyEqual, AllowDuplicates>::operator!=(const HashTable& other) const
{
    return !(*this, other);
}

template<typename K, typename M, typename H, typename E, bool D>
inline void swap(HashTable<K, M, H, E, D>& lhs, HashTable<K, M, H, E, D>& rhs) noexcept
{
    lhs.swap(rhs);
}

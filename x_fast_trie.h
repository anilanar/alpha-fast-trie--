//
//  x_fast_trie.h
//
//  X-fast-trie implementation based on https://github.com/vosen/kora
//  Author: Anil Anar.
//

#ifndef _x_fast_trie_h
#define _x_fast_trie_h

#include <unordered_map>
#include <utility>
#include <initializer_list>
#include <memory>

namespace kora {
    template<class KeyT, int Width, class ValueT, class Allocator = std::allocator<std::pair<const KeyT, ValueT>>>
    class x_fast_trie {
    private:
        struct x_fast_node;
        struct x_leaf_node;
        template<bool IsConst>
        class x_fast_trie_iterator;
        class x_fast_trie_const_iterator;
        
        typedef std::unordered_map<KeyT, x_fast_node> lookup_t;
        typedef typename std::allocator_traits<Allocator>::template rebind_alloc<x_leaf_node> node_allocator_t;
        node_allocator_t _allocator;
        typedef typename std::allocator_traits<node_allocator_t>::pointer x_leaf_node_ptr;
        
        size_t _count;
        int _width;
        int _version;
        
        lookup_t _table[Width];
        x_leaf_node* _leaf_list;
        
        x_fast_node* bottom(KeyT key);
        void insert_leaf_after(x_fast_node* marker, x_leaf_node* new_leaf);
        x_leaf_node* lower_node_from_bottom(x_fast_node *bottom, KeyT key);
        x_leaf_node* lower_node(KeyT key);
        x_leaf_node* higher_node(KeyT key);
        void remove_leaf(x_leaf_node leaf);
        
    public:
        typedef std::pair<const KeyT, ValueT>   value_type;
        typedef x_fast_trie_iterator<false>     iterator;
        typedef x_fast_trie_const_iterator      const_iterator;
        
        x_fast_trie();
        virtual ~x_fast_trie();
        
        ValueT& at(const KeyT& key);
        const ValueT& at(const KeyT& key) const;
        
        ValueT& operator[](const KeyT& key);
        ValueT& operator[](KeyT&& key);
        
        iterator begin();
        iterator end();
        const_iterator cbegin() const;
        const_iterator cend() const;
        
        iterator rbegin();
        iterator rend();
        const_iterator rcbegin() const;
        const_iterator rcend() const;
        
        bool empty() const;
        size_t size() const;
        size_t max_size() const;
        
        void clear();
        
        std::pair<iterator, bool> insert(const value_type& value);
        template<class InputIt>
        void insert(InputIt first, InputIt last);
        void insert(std::initializer_list<value_type> ilist);
        
        iterator    erase(const_iterator pos);
        iterator    erase(const_iterator first, const_iterator last);
        size_t      erase(const KeyT& key);
        
        size_t count();
        iterator find(const KeyT& key);
        const_iterator find(const KeyT& key) const;
        std::pair<iterator, iterator> equal_range(const KeyT& key);
        std::pair<const_iterator, const_iterator> equal_range(const KeyT& key) const;
        
        iterator lower_bound(const KeyT& key);
        const_iterator lower_bound(const KeyT& key) const;
        
        iterator upper_bound(const KeyT& key);
        const_iterator upper_bound(const KeyT& key) const;
    };
}

#include "x_fast_trie_impl.h"

#endif

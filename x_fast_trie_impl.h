//
//  x_fast_trie_impl.h
//
//  X-fast-trie implementation based on https://github.com/vosen/kora
//  Author: Anil Anar.
//

#ifndef _x_fast_trie_impl_h
#define _x_fast_trie_impl_h

#define __TMPL      template<class KeyT, int Width, class ValueT, class Allocator>
#define __CLS       kora::x_fast_trie<KeyT, Width, ValueT, Allocator>
#define __INNER     typename __CLS

#include <stdexcept>
#include <iterator>
#include <type_traits>

__TMPL
class __CLS::x_fast_trie_const_iterator: public x_fast_trie_iterator<true> {
private:
    typedef x_fast_trie_iterator<true> super;
public:
    x_fast_trie_const_iterator(const x_fast_trie_iterator<false> it): x_fast_trie_iterator<true>(it._leaf_list, it._node) {}
};

__TMPL
kora::x_fast_trie<KeyT, Width, ValueT, Allocator>::x_fast_trie():
_width(Width),
_count(0),
_version(0),
_leaf_list(0) {
}

__TMPL
__CLS::~x_fast_trie() {
    iterator it = begin();
    while(it != end()) {
        x_leaf_node *node = it._node;
        it++;
        _allocator.destroy(node);
        _allocator.deallocate(node, 1);
    }
}

__TMPL
ValueT& __CLS::at(const KeyT& key) {
    iterator it = find(key);
    if(it != end())
        return (*it).second;
    throw std::out_of_range("Specified key does not exist.");
}

__TMPL
const ValueT& __CLS::at(const KeyT& key) const {
    const_iterator it = find(key);
    if(it != cend())
        return (*it)->value();
    throw std::out_of_range("Specified key does not exist.");
}

__TMPL
ValueT& __CLS::operator[](const KeyT& key) {
    
    try {
        return at(key);
    } catch(std::out_of_range e) {
        std::pair<iterator, bool> r = insert({key, ValueT()});
        return (*(r.first)).second;
    }
}

__TMPL
ValueT& __CLS::operator[](KeyT&& key) {
    try {
        return at(key);
    } catch(std::out_of_range e) {
        std::pair<iterator, bool> r = insert({std::move(key), ValueT()});
        return (*(r.first)).second;
    }
}

__TMPL
__INNER::const_iterator __CLS::cbegin() const {
    if(!_leaf_list)
        return cend();
    return __CLS::const_iterator(_leaf_list, _leaf_list);
}

__TMPL
__INNER::const_iterator __CLS::cend() const {
    return const_iterator(_leaf_list, NULL);
}

__TMPL
__INNER::iterator __CLS::begin() {
    if(!_leaf_list)
        return end();
    return __CLS::iterator(_leaf_list, _leaf_list);
}

__TMPL
__INNER::iterator __CLS::end() {
    return __CLS::iterator(_leaf_list, NULL);
}

__TMPL
bool __CLS::empty() const {
    return _count == 0;
}

__TMPL
size_t __CLS::size() const {
    return _count;
}

__TMPL
size_t __CLS::max_size() const {
    KeyT zero = 0;
    return ~zero;
}

__TMPL
void __CLS::clear() {
    iterator it = begin();
    while(it != end()) {
        x_leaf_node *node = it._node;
        it++;
        _allocator.destroy(node);
        _allocator.deallocate(node);
    }
    _count = 0;
    _version = 0;
    _leaf_list = NULL;
    for(int i = 0; i < _width; i++) {
        _table[i].clear();
    }
}

__TMPL
std::pair<__INNER::iterator, bool> __CLS::insert(const value_type &value) {
    KeyT key = value.first;
    x_fast_node *bottom_ = bottom(key);
    x_leaf_node *predecessor = lower_node_from_bottom(bottom_, key);
    x_leaf_node *pred_right;
    if(predecessor)
        pred_right = (x_leaf_node *)(predecessor->right);
    else
        pred_right = _leaf_list;
    if(pred_right && pred_right->key() == key)
        return { iterator(_leaf_list, pred_right), false };
    
    _count++;
    _version++;
    x_leaf_node_ptr end_node = _allocator.allocate(1);
    _allocator.construct(end_node, value);
    insert_leaf_after(predecessor, end_node);
    if(!bottom_) {
        x_fast_node new_fast_node = {end_node, end_node};
        _table[0].insert({0, new_fast_node});
    }
    
    x_fast_node *old_node = NULL;
    x_fast_node *current;
    
    for(int i = 0; i < _width; i++) {
        KeyT id_ = key >> (_width - 1 - i) >> 1;
        lookup_t& table = _table[i];
        typename lookup_t::iterator current_it = table.find(id_);
        if(current_it != table.end()) {
            current = &(current_it->second);
            x_leaf_node *leaf = NULL;
            if(current->left && current->left->is_leaf())
                leaf = dynamic_cast<x_leaf_node *>(current->left);
            
            if(leaf && leaf->key() > key)
                current->left = end_node;
            else {
                leaf = dynamic_cast<x_leaf_node *>(current->right);
                if(leaf && leaf->key() < key)
                    current->right = end_node;
            }
        } else if(old_node) {
            x_fast_node new_node = { end_node, end_node };
            current = &(*((table.insert({id_, new_node})).first)).second;
            if(id_ & 1)
                old_node->right = current;
            else
                old_node->left = current;
        }
        else throw std::logic_error("Could not find a proper non-leaf node and a non-leaf node could not be created to link with. File a bug report.");
        old_node = current;
    }
    
    return { iterator(_leaf_list, end_node), true };
}

__TMPL
template<class InputIt>
void __CLS::insert(InputIt first, InputIt last) {
    for(;first != last; first++) {
        insert(*first);
    }
}

__TMPL
void __CLS::insert(std::initializer_list<value_type> ilist) {
    for(const value_type* it = ilist.begin(); it != ilist.end(); it++) {
        insert(*it);
    }
}

__TMPL
__INNER::iterator __CLS::erase(const_iterator pos) {
    x_leaf_node *leaf = pos._node;
    KeyT key = leaf->key();
    x_leaf_node *right = (x_leaf_node *)leaf->right;
    x_leaf_node *left = (x_leaf_node *)leaf->left;
    if(right == leaf)
        _leaf_list = NULL;
    else {
        leaf->left->right = right;
        right->left = leaf->left;
        if(leaf == _leaf_list)
            _leaf_list = right;
    }
    
    bool single = true;
    for(int i = _width - 1; i >= 0; i--) {
        KeyT id_ = key >> (_width - 1 - i) >> 1;
        bool is_from_right = ((key >> (_width - 1 - i)) & 1) == 1;
        typename lookup_t::iterator current_it = _table[i].find(id_);
        x_fast_node &current = current_it->second;
        if(single) {
            if(is_from_right && (!dynamic_cast<x_leaf_node *>(current.left) || (i == (_width - 1) && ((x_leaf_node *)(current.left))->key() != key))) {
                
                current.right = left;
                single = false;
            }
            else if(!is_from_right && (!dynamic_cast<x_leaf_node *>(current.right) || (i == (_width - 1) && ((x_leaf_node *)(current.right))->key() != key))) {
                current.left = right;
                single = false;
            }
            else {
                _table[i].erase(current_it);
            }
        }
        else {
            if(current.left == leaf)
                current.left = right;
            else
                current.right = left;
        }
    }
    
    _count--;
    _version++;
    _allocator.destroy(leaf);
    _allocator.deallocate(leaf, 1);
    return iterator(_leaf_list, right);
}

__TMPL
__INNER::iterator __CLS::erase(const_iterator first, const_iterator last) {
    while(first != last && first != cend()) {
        first = erase(first);
    }
    
    return first;
}

__TMPL
size_t __CLS::count() {
    return _count;
}

__TMPL
__INNER::iterator __CLS::find(const KeyT &key) {
    lookup_t& table = _table[_width - 1];
    __INNER::lookup_t::iterator node_it = table.find(key >> 1);
    if(node_it != table.end()) {
        x_fast_node &node = node_it->second;
        if((key & 1) == 1) {
            x_leaf_node *right_ptr = (x_leaf_node *)node.right;
            if(right_ptr && right_ptr->key() == key)
                return iterator(_leaf_list, right_ptr);
        } else {
            x_leaf_node *left_ptr = (x_leaf_node *)node.left;
            if(left_ptr && left_ptr->key() == key)
                return iterator(_leaf_list, left_ptr);
        }
    }
    
    return end();
}

__TMPL
__INNER::const_iterator __CLS::find(const KeyT &key) const {
    const lookup_t& lookup = _table[_width - 1];
    __INNER::lookup_t::const_iterator node_it = lookup.find(key >> 1);
    if(node_it != lookup.end()) {
        const x_fast_node &node = (*node_it).second;
        if((key & 1) == 1) {
            const x_leaf_node *right_ptr = (x_leaf_node *)node.right;
            if(right_ptr && right_ptr->key == key) {
                return const_iterator(right_ptr);
            }
        } else {
            const x_leaf_node *left_ptr = (x_leaf_node *)node.left;
            if(left_ptr && left_ptr->key() == key)
                return const_iterator(_leaf_list, left_ptr);
        }
    }
    
    return cend();
}

__TMPL
__INNER::x_fast_node* __CLS::bottom(KeyT key) {
    int l = 0;
    int h = _width;
    x_fast_node *correct_node_ptr = NULL;
    do {
        int j = (l + h) / 2;
        const KeyT ancestor = key >> (_width - 1 - j) >> 1;
        lookup_t& table = _table[j];
        auto temp_node_it = table.find(ancestor);
        if(temp_node_it != table.end()) {
            l = j + 1;
            correct_node_ptr = &((*temp_node_it).second);
        } else {
            h = j;
        }
    } while(l < h);
    return correct_node_ptr;
}

__TMPL
void __CLS::insert_leaf_after(x_fast_node *marker, x_leaf_node *new_leaf) {
    if(marker == NULL) {
        if(_leaf_list == NULL) {
            _leaf_list = new_leaf;
            new_leaf->left = new_leaf;
            new_leaf->right = new_leaf;
        } else {
            _leaf_list->left->right = new_leaf;
            new_leaf->left = _leaf_list->left;
            new_leaf->right = _leaf_list;
            _leaf_list->left = new_leaf;
            _leaf_list = new_leaf;
        }
    } else {
        x_fast_node *right_node = marker->right;
        marker->right = new_leaf;
        new_leaf->left = marker;
        new_leaf->right = right_node;
        right_node->left = new_leaf;
    }
}

__TMPL
__INNER::x_leaf_node* __CLS::lower_node_from_bottom(x_fast_node *bottom, KeyT key) {
    if(!bottom)
        return NULL;
    
    x_leaf_node *leaf = dynamic_cast<x_leaf_node *>(bottom->right);
    if(leaf && leaf->key() < key)
        return leaf;
    leaf = dynamic_cast<x_leaf_node *>(bottom->left);
    if(leaf && leaf->key() < key)
        return leaf;
    leaf = dynamic_cast<x_leaf_node *>(bottom->left->left);
    if(leaf && leaf->key() < key)
        return leaf;
    return NULL;
}

__TMPL
__INNER::x_leaf_node* __CLS::lower_node(KeyT key) {
    x_fast_node *ancestor = bottom(key);
    return lower_node_from_bottom(ancestor, key);
}

__TMPL
__INNER::x_leaf_node* __CLS::higher_node(KeyT key) {
    x_fast_node *ancestor = bottom(key);
    if(!ancestor)
        return NULL;
    x_leaf_node *leaf = dynamic_cast<x_leaf_node *>(ancestor->left);
    if(leaf && leaf->key > key)
        return leaf;
    leaf = dynamic_cast<x_leaf_node *>(ancestor->right);
    if(leaf && leaf->key > key)
        return leaf;
    leaf = dynamic_cast<x_leaf_node *>(ancestor->right->right);
    if(leaf && leaf->key > key)
        return leaf;
    return NULL;
}

__TMPL
struct __CLS::x_fast_node {
    x_fast_node* left;
    x_fast_node* right;
    
    x_fast_node() {
        left = NULL;
        right = NULL;
    }
    
    x_fast_node(x_fast_node *l, x_fast_node *r) {
        left = l;
        right = r;
    }
    
    virtual bool is_leaf() { return false; }
};

__TMPL
struct __CLS::x_leaf_node: public __CLS::x_fast_node {
    std::pair<const KeyT, ValueT> key_value;
    
    x_leaf_node(const value_type& value): x_fast_node(), key_value(value)
    {}
    
    virtual bool is_leaf() { return true; }
    const KeyT& key() const { return key_value.first; }
    ValueT& value() { return key_value.second; }
    const ValueT& value() const { return key_value.second; }
};

__TMPL
template<bool IsConst>
class __CLS::x_fast_trie_iterator : public std::iterator<std::bidirectional_iterator_tag, value_type, size_t> {
    
protected:
    typedef typename std::conditional<IsConst, const value_type, value_type>::type ValueTypeT;
    
    friend class __CLS;
    __CLS::x_leaf_node *_node;
    __CLS::x_leaf_node *_leaf_list;
    x_fast_trie_iterator(x_leaf_node* leaf_list, x_leaf_node* node) {
        _node = node;
        _leaf_list = leaf_list;
    }
public:
    ValueTypeT& operator*() const { return _node->key_value; }
    ValueTypeT* operator->() const { return &(_node->key_value); }
    const x_fast_trie_iterator<IsConst>& operator++() {
        _node = (x_leaf_node *)_node->right;
        if(_node == _leaf_list) _node = NULL;
        return *this;
    }
    x_fast_trie_iterator<IsConst> operator++(int) {
        ++(*this);
        return *this;
    }
    const x_fast_trie_iterator<IsConst>& operator--() {
        _node = (x_leaf_node *)_node->left;
        if(_node == _leaf_list->left) _node = NULL;
        return *this;
    }
    x_fast_trie_iterator<IsConst> operator--(int) {
        --(*this);
        return *this;
    }
    bool operator==(const x_fast_trie_iterator<IsConst>& other) const {
        return _node == other._node;
    }
    bool operator!=(const x_fast_trie_iterator<IsConst>& other) const {
        return _node != other._node;
    }
};





#undef __INNER
#undef __CLS
#undef __TMPL
#endif

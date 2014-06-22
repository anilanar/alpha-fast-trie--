//
//  x_fast_trie.cpp
//  fast-trie-unit-tests
//
//  Created by Anil Anar on 20.06.2014.
//  Copyright (c) 2014 Urnware. All rights reserved.
//

#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#define private protected

#include "x_fast_trie.h"

template<class KeyT, int Width, class ValueT, class Allocator = std::allocator<std::pair<const KeyT, ValueT>>>

class x_fast_trie_test: public kora::x_fast_trie<KeyT, Width, ValueT, Allocator> {
private:
    typedef kora::x_fast_trie<unsigned int, 32, std::string> super;
public:
    void verify() {
        std::unordered_set<KeyT> levels[Width];
        std::set<KeyT> nodes;
        for(super::iterator it = super::begin(); it != super::end(); it++)
            nodes.insert(it->first);
        
        super:: x_fast_node *temp;
        for(auto node : nodes) {
            for(int i = 0; i < super::_width; i++) {
                KeyT id_ = node >> (super::_width - 1 - i) >> 1;
                super::lookup_t &lookup = super::_table[i];
                super::lookup_t::iterator temp_it = lookup.find(id_);
                if(temp_it == lookup.end())
                    throw std::exception();
                temp = &((*temp_it).second);
                if(temp->left->is_leaf() && !nodes.count(dynamic_cast<super::x_leaf_node *>(temp->left)->key()))
                    throw std::exception();
                if(temp->right->is_leaf() && !nodes.count(dynamic_cast<super::x_leaf_node *>(temp->right)->key()))
                    throw std::exception();
                if(i == super::_width - 1 && (!(temp->left->is_leaf()) || !(temp->right->is_leaf())))
                    throw std::exception();
                levels[i].insert(id_);
            }
        }
        for(int i = 0; i < super::_width; i++) {
            if(super::_table[i].size() != levels[i].size())
                throw std::exception();
        }
    }
};

typedef std::pair<const unsigned int, std::string> value_type;
typedef x_fast_trie_test<unsigned int, 32, std::string> trie_type;

bool cmp(const value_type &a, const value_type &b) {
    return a.first < b.first;
}

class x_fast_trie: public testing::Test {
};



TEST_F(x_fast_trie, Creation) {
    x_fast_trie_test<unsigned int, 32, std::string> tree;
}

TEST_F(x_fast_trie, SimpleAddition) {
    x_fast_trie_test<unsigned int, 32, std::string> tree;
    tree.insert({1, "1"});
}

TEST_F(x_fast_trie, AddMultiple) {
    x_fast_trie_test<unsigned int, 32, std::string> tree;
    tree.insert({1, "1"});
    tree.insert({1, "1"});
    tree.insert({2, "2"});
    tree.insert({31, "31"});
    tree.insert({3, "3"});
    tree.insert({4, "4"});
    tree.insert({5, "5"});
    tree.insert({6, "6"});
    tree.insert({27, "27"});
    tree.insert({21, "21"});
    tree.insert({19, "19"});
    EXPECT_EQ(tree[1], "1");
    EXPECT_EQ(tree[2], "2");
    EXPECT_EQ(tree[31], "31");
    EXPECT_EQ(tree[3], "3");
    EXPECT_EQ(tree[4], "4");
    EXPECT_EQ(tree[5], "5");
    EXPECT_EQ(tree[6], "6");
    EXPECT_EQ(tree[27], "27");
    EXPECT_EQ(tree[21], "21");
    EXPECT_EQ(tree[19], "19");
}

TEST_F(x_fast_trie, Deletion) {
    x_fast_trie_test<unsigned int, 32, std::string> trie;
    std::string temp;
    EXPECT_TRUE(trie.insert({0, "0"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({1, "1"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_NE(trie.find(0), trie.end());
    trie.erase(trie.find(0));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_EQ(trie.find(0), trie.end());
    EXPECT_NE(trie.find(1), trie.end());
    EXPECT_EQ(trie.find(1)->second, "1");
    trie.erase(trie.find(1));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({110, "110"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_NE(trie.find(110), trie.end());
    trie.erase(trie.find(110));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_EQ(trie.find(110), trie.end());
    EXPECT_TRUE(trie.insert({270, "270"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({182, "182"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({180, "180"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({184, "184"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({40, "40"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({200, "200"}).second);
    EXPECT_NO_THROW(trie.verify());
    EXPECT_TRUE(trie.insert({461, "461"}).second);
    EXPECT_TRUE(trie.insert({158, "158"}).second);
    EXPECT_TRUE(trie.insert({763, "763"}).second);
    EXPECT_EQ(trie.find(370), trie.end());
    EXPECT_EQ(trie.find(110), trie.end());
    EXPECT_NE(trie.find(461), trie.end());
    trie.erase(trie.find(461));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_EQ(trie.find(461), trie.end());
    EXPECT_THROW(trie.at(461), std::out_of_range);
    EXPECT_EQ(trie.at(184), "184");
    trie.erase(trie.find(184));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_THROW(trie.at(184), std::out_of_range);
    EXPECT_EQ(trie.at(763), "763");
    trie.erase(trie.find(763));
    EXPECT_NO_THROW(trie.verify());
    EXPECT_THROW(trie.at(763), std::out_of_range);
}

TEST_F(x_fast_trie, Iteration) {
    x_fast_trie_test<unsigned int, 32, std::string> trie;
    EXPECT_EQ(trie.begin(), trie.end());
    std::pair<unsigned int, std::string> pairs[] = {
        {699, "699"},
        {477, "477"},
        {840, "840"},
        {324, "324"},
        {750, "750"},
        {751, "751"},
        {563, "563"},
        {913, "913"},
    };
    
    for(auto p : pairs)
        trie.insert(p);
    
    std::sort(pairs, pairs + 8, cmp);
    
    int i = 0;
    for(auto it = trie.begin(); i < trie.size(); it++, i++) {
        EXPECT_EQ(it->first, pairs[i].first);
    }
}



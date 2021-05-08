/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#include<ctime>
#include<cstdlib>
#include <iostream>

namespace sjtu {

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
    private:
        class Node {
        public:
            Node *left = nullptr, *right = nullptr;
            value_type val;
            int size, priority;

            Node() : size(0), left(nullptr), right(nullptr) {//todo:avoid using
                priority = rand();
            }

            Node(const value_type &v) : size(1), val(v), left(nullptr), right(nullptr) {
                priority = rand();
            }

            ~Node() {
                left = right = nullptr;
                size = 0;
            }

            void update() {
                size = 1 + (left != nullptr ? left->size : 0) + (right != nullptr ? right->size : 0);
            }
        };

        class Treap {
        public:
            Node *root;
            int size;/////fake_size
            Compare cmp;

            Treap() : root(nullptr), size(0) {////todo:avoid using
            }

            Treap(const value_type &v) : root(nullptr) {
                root = new Node(v);
                size = root->size;
            }

            Treap(const Treap &other) : root(nullptr) {
                if (other.root) {
                    root = new_node(other.root);
                    size = root->size;
                }
            }

            Treap &operator=(const Treap &other) {
                if (this == &other) return *this;
                root = new_node(other);
                size = root->size;
                return *this;
            }

            void clear(Node *node) {
                if (node == nullptr) return;
                if (node->right)
                    clear(node->right);
                if (node->left) clear(node->left);
                delete node;
            }

            ~Treap() {
                clear(root);
                root = nullptr;
                size = 0;
            }

            Node *new_node(Node *other) {
                Node *node = new Node(other->val);
                if (other->left) node->left = new_node(other->left);
                if (other->right) node->right = new_node(other->right);
                node->size = other->size;
                return node;
            }

            Node *merge(Node *aa, Node *bb) {
                if (!aa) return bb;
                if (!bb) return aa;
                if (aa->priority < bb->priority) {
                    aa->right = merge(aa->right, bb);
                    aa->update();
                    return aa;
                } else {
                    bb->left = merge(aa, bb->left);
                    bb->update();
                    return bb;
                }
            }

            pair<Node *, Node *> split(Node *pos, const int &kk) {
                if (!pos) return pair<Node *, Node *>(nullptr, nullptr);
                if (kk == 0) return pair<Node *, Node *>(nullptr, pos);
                if (!pos->left && !pos->right) return pair<Node *, Node *>(pos, nullptr);

                if (pos->left && pos->left->size >= kk) {
                    pair<Node *, Node *> tmp = split(pos->left, kk);
                    pos->left = tmp.second;
                    pos->update();
                    tmp.second = pos;
                    return tmp;
                } else {
                    pair<Node *, Node *> tmp = split(pos->right, kk - 1 - (pos->left ? pos->left->size : 0));
                    pos->right = tmp.first;
                    pos->update();
                    tmp.first = pos;
                    return tmp;
                }
            }

            bool exist(Node *pos, const Key &key) const {
                if (pos == nullptr) return false;
                if (!cmp(pos->val.first, key) && !cmp(key, pos->val.first)) return true;
                return cmp(key, pos->val.first) ? exist(pos->left, key) : exist(pos->right, key);
            }

            int get_rank(Node *pos, const Key &key) const {
                if (pos == nullptr) return 0;
                int lsize = pos->left ? pos->left->size : 0;
                return cmp(key, pos->val.first) ? get_rank(pos->left, key) : get_rank(pos->right, key) + 1 +
                                                                             lsize;
            }

            Node *get_kth(const int &k) {
                pair<Node *, Node *> x = split(root, k - 1);
                pair<Node *, Node *> y = split(x.second, 1);
                Node *ans = y.first;
                root = merge(x.first, merge(ans, y.second));
                return ans;
            }

            Node *insert(const value_type &val) {
                if (root == nullptr) {
                    root = new Node(val);
                    size = 1;
                    return root;
                }
                int k = get_rank(root, val.first);
                pair<Node *, Node *> x(split(root, k));
                Node *pos = new Node(val);
                root = merge(x.first, merge(pos, x.second));
                return pos;
            }

            void remove(const Key &key) {
                int k = get_rank(root, key);
                Node *node = get_kth(k);
                pair<Node *, Node *> x = split(root, k - 1);
                pair<Node *, Node *> y = split(x.second, 1);
                root = merge(x.first, y.second);
                delete node;
            }

            int sze() {
                return root ? root->size : 0;
            }

            void print() {//todo
                for (int i = 1; i <= sze(); ++i) {
                    Node *node = get_kth(i);
                    std::cout << sze() << ' ' << node->val.first.counter << ' ' << node->val.first.val << ' '
                              << node->val.second
                              << std::endl;
                }
            }
        };
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
    private:
        Treap *treap;
        Compare cmp;
    public:
        class const_iterator;

        class iterator {
            friend class map<Key, T, Compare>;

        private:
            map<Key, T, Compare> *map_ptr;
            Treap *treap_ptr;
            Node *node_ptr;
            /**
             * TODO add data members
             *   just add whatever you want.
             */
        public:
            iterator() : map_ptr(nullptr), treap_ptr(nullptr), node_ptr(nullptr) {}

            iterator(const iterator &other) : map_ptr(other.map_ptr), treap_ptr(other.treap_ptr),
                                              node_ptr(other.node_ptr) {}

            iterator(map<Key, T, Compare> *map, Node *node) : map_ptr(map), treap_ptr(map->treap), node_ptr(node) {}

            iterator &operator=(const iterator &other) {
                if (this == &other) return *this;
                map_ptr = other.map_ptr;
                treap_ptr = other.treap_ptr;
                node_ptr = other.node_ptr;
                return *this;
            }

            iterator operator++(int) {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (treap_ptr->sze() < k) throw invalid_iterator();
                iterator iter = *this;
                if (treap_ptr->sze() == k) node_ptr = nullptr;
                node_ptr = treap_ptr->get_kth(k + 1);
                return iter;
            }

            /**
             * TODO ++iter
             */
            iterator &operator++() {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (treap_ptr->sze() < k) throw invalid_iterator();
                if (treap_ptr->sze() == k) node_ptr = nullptr;
                node_ptr = treap_ptr->get_kth(k + 1);
                return *this;
            }

            /**
             * TODO iter--
             */
            iterator operator--(int) {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                if (node_ptr == nullptr) return iterator(map_ptr, treap_ptr->get_kth(treap_ptr->sze()));
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (k <= 1) throw invalid_iterator();
                iterator iter = *this;
                node_ptr = treap_ptr->get_kth(--k);
                return iter;
            }

            /**
             * TODO --iter
             */
            iterator &operator--() {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                if (node_ptr == nullptr) {
                    node_ptr = treap_ptr->get_kth(treap_ptr->sze());
                    return *this;
                }
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (k <= 1) throw invalid_iterator();
                node_ptr = treap_ptr->get_kth(--k);
                return *this;
            }

            /**
             * an operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const {
                if (node_ptr == nullptr) throw invalid_iterator();
                return node_ptr->val;
            }

            bool operator==(const iterator &rhs) const {
                if (map_ptr != rhs.map_ptr) return false;
                if (node_ptr != rhs.node_ptr) return false;
                return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (map_ptr != rhs.map_ptr) return false;
                if (node_ptr != rhs.node_ptr) return false;
                return true;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept {
                if (node_ptr == nullptr) return nullptr;
                return &(node_ptr->val);
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend class map<Key, T, Compare>;

        private:// data members.
            const map<Key, T, Compare> *map_ptr;
            Treap *treap_ptr;//todo:add const
            const Node *node_ptr;
        public:
            const_iterator() : map_ptr(nullptr), treap_ptr(nullptr), node_ptr(nullptr) {}

            const_iterator(const iterator &other) : map_ptr(other.map_ptr), treap_ptr(other.treap_ptr),
                                                    node_ptr(other.node_ptr) {}

            const_iterator(const const_iterator &other) : map_ptr(other.map_ptr), treap_ptr(other.treap_ptr),
                                                          node_ptr(other.node_ptr) {}

            const_iterator(const map<Key, T, Compare> *map, Node *node) : map_ptr(map), treap_ptr(map->treap),
                                                                          node_ptr(node) {}

            const_iterator &operator=(const const_iterator &other) {
                if (this == &other) return *this;
                map_ptr = other.map_ptr;
                treap_ptr = other.treap_ptr;
                node_ptr = other.node_ptr;
                return *this;
            }

            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.
            const_iterator operator++(int) {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (treap_ptr->sze() < k) throw invalid_iterator();
                const_iterator iter = *this;
                if (treap_ptr->sze() == k) node_ptr = nullptr;
                node_ptr = treap_ptr->get_kth(++k);
                return iter;
            }

            /**
             * TODO ++iter
             */
            const_iterator &operator++() {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (treap_ptr->sze() < k) throw invalid_iterator();
                if (treap_ptr->sze() == k) node_ptr = nullptr;
                node_ptr = treap_ptr->get_kth(++k);
                return *this;
            }

            /**
             * TODO iter--
             */
            const_iterator operator--(int) {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                if (node_ptr == nullptr) return const_iterator(map_ptr, treap_ptr->get_kth(treap_ptr->sze()));
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (k == 1) throw invalid_iterator();
                const_iterator iter = *this;
                node_ptr = treap_ptr->get_kth(--k);
                return iter;
            }

            /**
             * TODO --iter
             */
            const_iterator &operator--() {
                treap_ptr = map_ptr->treap;
                if (treap_ptr == nullptr) throw invalid_iterator();
                if (treap_ptr->sze() == 0) throw invalid_iterator();
                if (node_ptr == nullptr) {
                    node_ptr = treap_ptr->get_kth(treap_ptr->sze());
                    return *this;
                }
                int k = treap_ptr->get_rank(treap_ptr->root, node_ptr->val.first);
                if (k == 1) throw invalid_iterator();
                node_ptr = treap_ptr->get_kth(--k);
                return *this;
            }

            /**
             * an operator to check whether two iterators are same (pointing to the same memory).
             */
            const value_type &operator*() const {
                if (node_ptr == nullptr) throw invalid_iterator();
                return node_ptr->val;
            }

            bool operator==(const iterator &rhs) const {
                if (map_ptr != rhs.map_ptr) return false;
                if (treap_ptr != rhs.treap_ptr) return false;
                if (node_ptr != rhs.node_ptr) return false;
                return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (map_ptr != rhs.map_ptr) return false;
                if (treap_ptr != rhs.treap_ptr) return false;
                if (node_ptr != rhs.node_ptr) return false;
                return true;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            const value_type *operator->() const noexcept {
                if (node_ptr == nullptr) return nullptr;
                return &(node_ptr->val);
            }
        };


        /**
         * TODO two constructors
         */
        map() : treap(nullptr) {
            treap = new Treap;
        }

        map(const map &other) : treap(nullptr) {
            treap = new Treap(*(other.treap));
        }

        map &operator=(const map &other) {
            if (this == &other) return *this;
            if (treap) delete treap;
            treap = new Treap(*(other.treap));
            return *this;
        }

        /**
         * TODO Destructors
         */
        ~map() {
            if (treap)
//                treap->clear(treap->root);
                delete treap;
        }

        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key) {
            if (!treap) throw container_is_empty();
            if (!treap->exist(treap->root, key)) throw index_out_of_bound();
            int k = treap->get_rank(treap->root, key);
            return treap->get_kth(k)->val.second;
        }

        const T &at(const Key &key) const {
            if (!treap) throw container_is_empty();
            if (!treap->exist(treap->root, key)) throw index_out_of_bound();
            int k = treap->get_rank(treap->root, key);
            return treap->get_kth(k)->val.second;
        }

        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T &operator[](const Key &key) {
            if (!treap) {
                treap = new Treap;
                treap->insert(value_type(key, T()));
            }
            if (!treap->exist(treap->root, key)) {
                treap->insert(value_type(key, T()));
            }
            int k = treap->get_rank(treap->root, key);
            return treap->get_kth(k)->val.second;
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const {
            if (!treap) throw container_is_empty();
            if (!treap->exist(treap->root, key)) throw index_out_of_bound();
            int k = treap->get_rank(treap->root, key);
            return treap->get_kth(k)->val.second;
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            if (treap == nullptr) return iterator(this, nullptr);
            return iterator(this, treap->get_kth(1));
        }

        const_iterator cbegin() const {
            if (treap == nullptr) return const_iterator(this, nullptr);
            return const_iterator(this, treap->get_kth(1));
        }

        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(this, nullptr);
        }

        const_iterator cend() const {
            return const_iterator(this, nullptr);
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            if (size()) return false;
            return true;
        }

        /**
         * returns the number of elements.
         */
        size_t size() const {
            return treap ? treap->sze() : 0;
        }

        /**
         * clears the contents
         */
        void clear() {
            if (treap)
//                treap->clear(treap->root);
                delete treap;
            treap = nullptr;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            if (!treap) treap = new Treap;
            iterator iter = find(value.first);
            if (iter == end()) return pair<iterator, bool>(iterator(this, treap->insert(value)), true);
            return pair<iterator, bool>(iter, false);
        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos == end()) throw invalid_iterator();
            if (pos.treap_ptr != treap) throw invalid_iterator();
            if (pos.node_ptr == nullptr) throw invalid_iterator();
            if (end() == find(pos.node_ptr->val.first)) throw invalid_iterator();
            treap->remove(pos.node_ptr->val.first);
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            if (find(key) == cend()) return 0;
            return 1;
        }

        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            if (treap == nullptr) return end();
            if (treap->exist(treap->root, key)) {
                int k = treap->get_rank(treap->root, key);
                return iterator(this, treap->get_kth(k));
            }
            return end();
        }

        const_iterator find(const Key &key) const {
            if (treap == nullptr) return cend();
            if (treap->exist(treap->root, key)) {
                int k = treap->get_rank(treap->root, key);
                return const_iterator(this, treap->get_kth(k));
            }
            return cend();
        }

        void print() {//todo
            treap->print();
        }
    };

}

#endif
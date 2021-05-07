#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"
#include "utility.hpp"
#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
    template<typename T>
    class vector {
        typedef T value_type;
        typedef value_type **pointer;
        typedef value_type *pointer_;
        typedef value_type &reference;

    private:
        pointer start;
        pointer finish;
        pointer end_of_storage;

    public:
        /**
         * TODO
         * a type for actions of the elements of a vector, and you should write
         *   a class named const_iterator with same interfaces.
         */
        /**
         * you can see RandomAccessIterator at CppReference for help.
         */
        class const_iterator;

        class iterator {
            friend vector<T>;
        private:
            const vector<T> *loc;

            T **iter;
        public:
            iterator(const iterator &other) : loc(other.loc), iter(other.iter) {}

            iterator(vector<T> *vec = nullptr, T **it = nullptr) : loc(vec), iter(it) {}

            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            iterator operator+(const int &n) const {
//                int offset = iter - loc->start + n;
//                if (offset > loc->finish - loc->start) throw invalid_iterator();
                iterator tmp = *this;
                tmp.iter += n;
                return tmp;
            }

            iterator operator-(const int &n) const {
//                int offset = iter - loc->start - n;
//                if (offset < 0) throw invalid_iterator();
                iterator tmp = *this;
                tmp.iter -= n;
                return tmp;
            }

            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const {
                if (rhs.loc != this->loc) throw invalid_iterator();
                return this->iter - rhs.iter;
            }

            iterator &operator+=(const int &n) {
                this->iter += n;
                return *this;
            }

            iterator &operator-=(const int &n) {
                this->iter -= n;
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                (*this) += 1;
                return tmp;
            }

            iterator &operator++() {
                return (*this) += 1;
            }

            iterator operator--(int) {
                iterator tmp = *this;
                (*this) -= 1;
                return tmp;
            }

            iterator &operator--() {
                return (*this) -= 1;
            }

            T &operator*() const {
                if (loc == nullptr || iter < loc->start || iter > loc->finish) throw invalid_iterator();
                return **iter;
            }

            bool operator==(const iterator &rhs) const {
                return (loc == rhs.loc) && (iter == rhs.iter);
            }

            bool operator==(const const_iterator &rhs) const {
                return (loc == rhs.loc) && (iter == rhs.iter);
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }
        };

        /**
         * has same function as iterator, just for a const object.
         */
        class const_iterator {
            friend vector<T>;
        private:
            const vector<T> *loc;

            T **iter;
        public:
            const_iterator(const const_iterator &other) : loc(other.loc), iter(other.iter) {}

            const_iterator(vector<T> *vec = nullptr, T **it = nullptr) : loc(vec), iter(it) {}

            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            const_iterator operator+(const int &n) const {
//                int offset = iter - loc->start + n;
//                if (offset > loc->finish - loc->start) throw invalid_iterator();
                iterator tmp = *this;
                tmp.iter += n;
                return tmp;
            }

            const_iterator operator-(const int &n) const {
//                int offset = iter - loc->start - n;
//                if (offset < 0) throw invalid_iterator();
                iterator tmp = *this;
                tmp.iter -= n;
                return tmp;
            }

            int operator-(const iterator &rhs) const {
                if (rhs.loc != this->loc) throw invalid_iterator();
                return this->iter - rhs.iter;
            }

            const_iterator &operator+=(const int &n) {
                this->iter += n;
                return *this;
            }

            const_iterator &operator-=(const int &n) {
                this->iter -= n;
                return *this;
            }

            const_iterator operator++(int) {
                iterator tmp = *this;
                (*this) += 1;
                return tmp;
            }

            const_iterator &operator++() {
                return (*this) += 1;
            }

            const_iterator operator--(int) {
                iterator tmp = *this;
                (*this) -= 1;
                return tmp;
            }

            const_iterator &operator--() {
                return (*this) -= 1;
            }

            const T &operator*() const {
                if (loc == nullptr || iter < loc->start || iter > loc->finish) throw invalid_iterator();
                return **iter;
            }

            bool operator==(const iterator &rhs) const {
                return (loc == rhs.loc) && (iter == rhs.iter);
            }

            bool operator==(const const_iterator &rhs) const {
                return (loc == rhs.loc) && (iter == rhs.iter);
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }
        };

    private:
        iterator create(int size = 1 << 4) {
            pointer tmp = new pointer_[size];
            if (start == nullptr) {
                start = tmp;
                finish = tmp - 1;
                end_of_storage = tmp + size - 1;
            } else {
                for (auto iter = start; iter != end_of_storage + 1; ++iter) {
                    tmp[iter - start] = *iter;
                    *iter = nullptr;
                }
                int old_size = capacity() + 1;
                destroy();
                finish = tmp + old_size - 1;
                start = tmp;
                end_of_storage = tmp + size - 1;
            }
            return iterator(this, start);
        }

        void construct(pointer pos, const T &x) {
            ++pos;
            *pos = new value_type(x);
        }

        void destroy() {
            if (start == nullptr) return;
            auto iter = start;
            while (finish - iter >= 0) {
                if (*iter != nullptr) delete *iter;
                ++iter;
            }
            delete[] start;
            start = nullptr;
        }

    public:
        /**
         * TODO Constructs
         * Atleast two: default constructor, copy constructor
         */
        vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}

        vector(const vector &other) {
            start = nullptr;
            create(other.capacity() + 1);
            finish = other.size() + start - 1;
            for (int i = 0; i < other.size(); ++i) start[i] = new value_type(*(other.start[i]));
        }

        /**
         * TODO Destructor
         */
        ~vector() {
            destroy();
        }

        /**
         * TODO Assignment operator
         */
        vector &operator=(const vector &other) {
            if (this == &other) return *this;
            destroy();
            create(other.capacity() + 1);
            finish = other.size() + start - 1;
            for (int i = 0; i < other.size(); ++i) start[i] = new value_type(*(other.start[i]));
            return *this;
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T &at(const size_t &pos) {
            if (pos < 0 || pos >= size()) throw index_out_of_bound();
            return **(start + pos);
        }

        const T &at(const size_t &pos) const {
            if (pos < 0 || pos >= size()) throw index_out_of_bound();
            return **(start + pos);
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        T &operator[](const size_t &pos) {
            if (pos < 0 || pos >= size()) throw index_out_of_bound();
            return **(start + pos);
        }

        const T &operator[](const size_t &pos) const {
            if (pos < 0 || pos >= size()) throw index_out_of_bound();
            return **(start + pos);
        }

        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T &front() const {
            if (finish - start == 0) throw container_is_empty();
            return **start;
        }

        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T &back() const {
            if (finish - start == 0) throw container_is_empty();
            return **finish;
        }

        /**
         * returns an iterator to the beginning.
         */
        iterator begin() {
            return iterator(this, start);
        }

        const_iterator cbegin() const {
            const_iterator const_iter;
            const_iter.loc = this;
            const_iter.iter = start;
            return const_iter;
        }

        /**
         * returns an iterator to the end.
         */
        iterator end() {
            return iterator(this, finish + 1);
        }

        const_iterator cend() const {
            const_iterator const_iter;
            const_iter.loc = this;
            const_iter.iter = finish + 1;
            return const_iter;
        }

        /**
         * checks whether the container is empty
         */
        bool empty() const {
            if (start == nullptr) return true;
            return start == finish + 1;
        }

        /**
         * returns the number of elements
         */
        size_t size() const {
            if (empty()) return 0;
            int size = finish - start + 1;
            return finish - start + 1;
        }

        size_t capacity() const {
            return end_of_storage - start;
        }

        /**
         * clears the contents
         */
        void clear() {
            for (auto iter = start; iter != finish + 1; ++iter) delete *iter;
            finish = start - 1;
        }

        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert(iterator pos, const T &value) {
            if (start == nullptr) create();
            int dis = pos - begin();
            if (finish == end_of_storage) {
                size_t old = size();
                create(2 * old);
//                const int old_size = size();
//                const int new_size = old_size != 0 ? 2 * old_size : 1;
//
//                vector<value_type> new_vector;
//                new_vector.create(new_size);
//                new_vector.finish = new_vector.start + size() - 1;
//                for (int i = 0; i < size(); ++i) new_vector.start[i] = new value_type(*(start[i]));
//                *this = new_vector;
            }
            /*start[size()] = new value_type(**finish);
//            construct(finish, **finish);
            ++finish;
            for (auto iter = iterator(this, finish - 1); iter != pos; --iter) *(iter + 1) = *iter;
            *pos = value;*/
//            for (auto iter = iterator(this, finish++); iter != pos; --iter) *(iter + 1) = *iter;
            for (int i = size() - 1; i >= dis; --i) start[i + 1] = start[i];
            ++finish;
            start[dis] = new value_type(value);
            return iterator(this, start + (pos - begin()));
        }

        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert(const size_t &ind, const T &value) {
            if (start == nullptr) create();
            if (ind > size()) throw index_out_of_bound();
            if (finish == end_of_storage) {
                create(2 * size());
//                vector<value_type> new_vector;
//                new_vector.create(2 * size());
//                new_vector.finish = new_vector.start + size() - 1;
//                for (int i = 0; i < size(); ++i) new_vector.start[i] = new value_type(*(start[i]));
//                *this = new_vector;
            }
            /*start[size()] = new value_type(back());
//            construct(finish, **finish);
            ++finish;
            for (int i = size() - 1; i > ind; --i) at(i) = at(i - 1);
            at(ind) = value;*/
            for (int i = size(); i > ind; --i) start[i] = start[i - 1];
            ++finish;
            start[ind] = new value_type(value);
            return iterator(this, start + ind);
        }

        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase(iterator pos) {
            pointer iter = start + (pos - begin());
            delete *iter;
            for (; iter != finish; ++iter) *iter = *(iter + 1);
            --finish;
            if (pos - begin() >= size()) return end();
            return iterator(this, start + (pos - begin()));
        }

        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const size_t &ind) {
            if (ind >= size()) throw index_out_of_bound();
            delete *(start + ind);
            for (int i = ind; i < size(); ++i) at(i) = at(i + 1);
            --finish;
            if (ind == size() - 1) return end();
            return start + ind;
        }

        /**
         * adds an element to the end.
         */
        void push_back(const T &value) {
            insert(size(), value);
        }

        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back() {
            if (size() == 0) throw container_is_empty();
            delete *finish;
            --finish;
        }
    };


}

#endif

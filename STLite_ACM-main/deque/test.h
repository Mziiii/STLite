#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
#define BLOCK_MAX_SIZE 320
#define MERGE_SIZE 320
#define MERGE_THRESHOLD 150
#define SPLIT_THRESHOLD 320

#include "exceptions.hpp"
#include <iostream>
#include <cstddef>

namespace sjtu {

    template<class T>
    class deque {
    private:
        class Element {
        public:
            T *data;
            Element *pre_ele, *nxt_ele;

            Element(const T &dat) : pre_ele(nullptr), nxt_ele(nullptr) { data = new T(dat); }

            Element(const Element &element) : pre_ele(nullptr), nxt_ele(nullptr) { data = new T(*(element.data)); }

            virtual ~Element() {
                if (data) delete data;
                data = nullptr;
            }
        };

        class Block {
        public:
            Block *pre_block, *nxt_block;
            Element *head_ele, *tail_ele;
            int size_block;

            Block() : pre_block(nullptr), nxt_block(nullptr), head_ele(nullptr), tail_ele(nullptr), size_block(0) {}

            void display() {
                int num = 0;
                std::cout << size_block << ' ';
                for (Element *ptr = head_ele; ptr; ptr = ptr->nxt_ele) {
                    std::cout << '*' << ' ';
                    ++num;
                }
                std::cout << (num == size_block) << '\n';
            }

            Block(const Block &block) : pre_block(block.pre_block), nxt_block(block.nxt_block),
                                        size_block(block.size_block), tail_ele(nullptr), head_ele(nullptr) {
                Element *tmp = block.head_ele;
                while (tmp) {
                    if (!tail_ele) {
                        head_ele = new Element(*tmp);
                        tail_ele = head_ele;
                    } else {
                        tail_ele->nxt_ele = new Element(*tmp);
                        tail_ele->nxt_ele->pre_ele = tail_ele;
                        tail_ele = tail_ele->nxt_ele;
                    }
                    tmp = tmp->nxt_ele;
                }
            }

            virtual ~Block() {
                Element *ele = nullptr;
                while (tail_ele) {
                    ele = tail_ele;
                    tail_ele = tail_ele->pre_ele;
                    delete ele;
                }
                size_block = 0;
                head_ele = tail_ele = nullptr;
                nxt_block = pre_block = nullptr;
            }

            Element *getEle(const int &ind) const {//0-base
                Element *ele = head_ele;
                for (int i = 0; i < ind; ++i) {
                    ele = ele->nxt_ele;
                }
                return ele;
            }

        public:
            void clear() {
                Element *ele = nullptr;
                while (head_ele) {
                    ele = head_ele;
                    head_ele = head_ele->nxt_ele;
                    //                    tail_ele = tail_ele->pre_ele;
                    delete ele;
                }
                size_block = 0;
                head_ele = tail_ele = nullptr;
                nxt_block = pre_block = nullptr;
            }

            void merge() {//调整被合并（nxt_block）的前后block关于block的信息，并将合并中间的元素连起来，删掉被合并的块
                if (nxt_block == nullptr) return;
                else if (nxt_block->size_block == 0) {}
                else {
                    size_block += nxt_block->size_block;
                    if (size_block == 0) {
                        head_ele = nxt_block->head_ele;
                        tail_ele = nxt_block->tail_ele;
                    } else {
                        nxt_block->head_ele->pre_ele = tail_ele;
                        tail_ele->nxt_ele = nxt_block->head_ele;
                        tail_ele = nxt_block->tail_ele;
                    }
                    nxt_block->tail_ele = nxt_block->head_ele = nullptr;
                }

                Block *temp = nxt_block;
                nxt_block = nxt_block->nxt_block;
                if (nxt_block) {
                    nxt_block->pre_block = this;
                }
                delete temp;
            }

            void split() {
                Block *new_block = new Block;
                new_block->size_block = size_block >> 1;
                new_block->pre_block = this;
                new_block->nxt_block = nxt_block;
                new_block->tail_ele = tail_ele;

                if (nxt_block) new_block->pre_block = new_block;
                nxt_block = new_block;
                size_block = size_block >> 1;
                new_block->head_ele = getEle(size_block);
                tail_ele = new_block->head_ele->pre_ele;
                new_block->head_ele->pre_ele = nullptr;
                tail_ele->nxt_ele = nullptr;
            }

            int erase(const int &ind) {//////////todo 1or0
                if (!size_block) return 0;
                if (ind >= size_block || ind < 0) {
//                    std::cout << "qwer132";///////todo
                    throw index_out_of_bound();
                }

                if (size_block == 1) {
                    delete tail_ele;
                    tail_ele = head_ele = nullptr;
                } else {
                    if (!ind) {
                        Element *element = head_ele;
                        head_ele = head_ele->nxt_ele;
                        head_ele->pre_ele = nullptr;
                        delete element;
                    } else if (ind == size_block - 1) {
                        Element *element = tail_ele;
                        tail_ele = tail_ele->pre_ele;
                        tail_ele->nxt_ele = nullptr;
                        delete element;
                    } else if (ind >= 1 && ind <= size_block - 2) {
                        Element *element = getEle(ind);
                        element->nxt_ele->pre_ele = element->pre_ele;
                        element->pre_ele->nxt_ele = element->nxt_ele;
                        delete element;
                    }
                }
                --size_block;
                if (nxt_block && size_block <= MERGE_THRESHOLD &&
                    size_block + nxt_block->size_block <= MERGE_SIZE) {
                    merge();
                    return 1;
                } else if (pre_block && size_block <= MERGE_THRESHOLD &&
                           size_block + pre_block->size_block <= MERGE_SIZE) {
                    pre_block->merge();
                    return 2;
                }
                return 0;
            }

            bool insert(const int &ind, Element *element) {///////////todo 1or0
                if (ind > size_block || ind < 0) {
//                    std::cout << "qwer168";///////todo
                    throw index_out_of_bound();
                }

                if (!size_block) { tail_ele = head_ele = element; }
                else {
                    if (!ind) {
                        element->nxt_ele = head_ele;
                        head_ele->pre_ele = element;
                        head_ele = element;
                    } else if (ind == size_block) {
                        tail_ele->nxt_ele = element;
                        element->pre_ele = tail_ele;
                        tail_ele = element;
                    } else if (ind >= 0 && ind < size_block) {
                        Element *ele = getEle(ind);
                        element->pre_ele = ele->pre_ele;
                        element->nxt_ele = ele;
                        ele->pre_ele->nxt_ele = element;
                        ele->pre_ele = element;
                    }
                }
                ++size_block;
                if (size_block >= SPLIT_THRESHOLD) {
                    split();
                    return true;
                }
                return false;
            }
        };

    private:
        int size_deque, num_block;
        Block *head_block, *tail_block;
    public:
        class const_iterator;

        class iterator {
            friend class deque<T>;

        private:
            /**
             * add data members
             *   just add whatever you want.
             */
            deque<T> *origin;
            Block *block;
            Element *ele;
            int ind_deque, ind_block;

        public:
            iterator() : origin(nullptr), block(nullptr), ele(nullptr), ind_block(0), ind_deque(0) {}

            iterator(const iterator &iter) : origin(iter.origin), block(iter.block), ele(iter.ele),
                                             ind_deque(iter.ind_deque), ind_block(iter.ind_block) {}

            iterator(int index_deque, deque<T> *deq) : origin(deq), ind_deque(index_deque) {
                if (index_deque > origin->size_deque)
                    throw invalid_iterator();
                if (index_deque == origin->size_deque) {
                    block = origin->tail_block;
                    if (block) ind_block = block->size_block;
                    ele = nullptr;
                } else {
                    ele = deq->getElement(ind_deque, block, ind_block);
                }
            }

            bool isValid() const {
                if (!ind_deque) return true;
                int index_block;
                Block *bb;
                Element *ee;
                ee = origin->getElement(ind_deque, bb, index_block);
                if (ee != ele || bb != block || index_block != ind_block) return false;
                return true;
            }

            /**
             * return a new iterator which pointer n-next elements
             *   if there are not enough elements, iterator becomes invalid
             * as well as operator-
             */
            iterator operator+(const int &n) const {
                iterator iter = *this;
                iter += n;
                return iter;
            }

            iterator operator-(const int &n) const {
                iterator iter = *this;
                iter -= n;
                return iter;
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const {
                if (origin != rhs.origin) throw invalid_iterator();//////todo
                return ind_deque - rhs.ind_deque;
            }

            int operator-(const const_iterator &rhs) const {
                if (origin != rhs.origin) throw invalid_iterator();//////todo
                return ind_deque - rhs.ind_deque;
            }

            iterator &operator+=(const int &n) {
                if (n < 0) return *this -= (-n);
                if (ind_deque + n > origin->size_deque) throw invalid_iterator();///todo
                ind_deque += n;
                ele = origin->getElement(ind_deque, block, ind_block);
                return *this;
            }

            iterator &operator-=(const int &n) {
                if (n < 0) return *this += (-n);
                if (ind_deque - n < 0) throw invalid_iterator();///todo
                ind_deque -= n;
                ele = origin->getElement(ind_deque, block, ind_block);
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                *this += 1;
                return tmp;
            }

            iterator &operator++() {
                *this += 1;
                return *this;
            }

            iterator operator--(int) {
                iterator tmp = *this;
                *this -= 1;
                return tmp;
            }

            iterator &operator--() {
                *this -= 1;
                return *this;
            }

            T &operator*() const {
                if (!ele || !(ele->data) || !isValid()) throw invalid_iterator();
                return *(ele->data);
            }

            T *operator->() const noexcept {
                if (!ele || !(ele->data) || !isValid()) throw invalid_iterator();
                return ele->data;
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                if (origin != rhs.origin) return false;
                if (ind_deque != rhs.ind_deque) return false;
                return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (origin != rhs.origin) return false;
                if (ind_deque != rhs.ind_deque) return false;
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

        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend class deque<T>;

        private:
            // data members.
            const deque<T> *origin;
            Block *block;
            Element *ele;
            int ind_deque, ind_block;
        public:
            const_iterator() : origin(nullptr), block(nullptr), ele(nullptr), ind_block(0), ind_deque(0) {}

            const_iterator(const const_iterator &other) : origin(other.origin), block(other.block), ele(other.ele),
                                                          ind_deque(other.ind_deque), ind_block(other.ind_block) {}

            const_iterator(const iterator &other) : origin(other.origin), block(other.block), ele(other.ele),
                                                    ind_deque(other.ind_deque), ind_block(other.ind_block) {}

            const_iterator(int index_deque, const deque<T> *deq) : origin(deq), ind_deque(index_deque) {
                if (index_deque > origin->size_deque)
                    throw invalid_iterator();
                if (index_deque == origin->size_deque) {
                    block = origin->tail_block;
                    ind_block = block->size_block;
                } else {
                    ele = deq->getElement(ind_deque, block, ind_block);
                }
            }

            bool isValid() const {
                int index_block;
                Block *bb;
                Element *ee;
                ee = origin->getElement(ind_deque, bb, index_block);
                if (ee != ele || bb != block || index_block != ind_block) return false;
                return true;
            }

            /**
             * return a new iterator which pointer n-next elements
             *   if there are not enough elements, iterator becomes invalid
             * as well as operator-
             */
            const_iterator operator+(const int &n) const {
                const_iterator iter = *this;
                iter += n;
                return iter;
            }

            const_iterator operator-(const int &n) const {
                const_iterator iter = *this;
                iter -= n;
                return iter;
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                if (origin != rhs.origin) throw invalid_iterator();//////todo
                return ind_deque - rhs.ind_deque;
            }

            int operator-(const iterator &rhs) const {
                if (origin != rhs.origin) throw invalid_iterator();//////todo
                return ind_deque - rhs.ind_deque;
            }

            const_iterator &operator+=(const int &n) {
                if (n < 0) return *this -= (-n);
                if (ind_deque + n > origin->size_deque) throw invalid_iterator();///todo
                ind_deque += n;
                ele = origin->getElement(ind_deque, block, ind_block);
                return *this;
            }

            const_iterator &operator-=(const int &n) {
                if (n < 0) return *this += (-n);
                if (ind_deque - n < 0) throw invalid_iterator();///todo
                ind_deque -= n;
                ele = origin->getElement(ind_deque, block, ind_block);
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator tmp = *this;
                *this += 1;
                return tmp;
            }

            const_iterator &operator++() {
                *this += 1;
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator tmp = *this;
                *this -= 1;
                return tmp;
            }

            const_iterator &operator--() {
                *this -= 1;
                return *this;
            }

            const T &operator*() const {
                if (!ele || !(ele->data) || !isValid()) throw invalid_iterator();
                return *(ele->data);
            }

            const T *operator->() const noexcept {
                if (!ele || !(ele->data) || !isValid()) throw invalid_iterator();
                return ele->data;
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                if (origin != rhs.origin) return false;
                if (ind_deque != rhs.ind_deque) return false;
                return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (origin != rhs.origin) return false;
                if (ind_deque != rhs.ind_deque) return false;
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
        };

        /**
         * Constructors
         */
        deque() : size_deque(0), num_block(1) {
            head_block = new Block();
            tail_block = head_block;
        }

        deque(const deque &other) : size_deque(other.size_deque), num_block(other.num_block), head_block(nullptr),
                                    tail_block(
                                            nullptr) {
            Block *block = other.head_block;
            while (block) {
                if (!head_block) {
                    tail_block = new Block(*block);
                    head_block = tail_block;
                } else {
                    tail_block->nxt_block = new Block(*block);
                    tail_block->nxt_block->pre_block = tail_block;
                    tail_block = tail_block->nxt_block;
                }
                block = block->nxt_block;
            }
        }

        void init() {
            size_deque = 0;
            num_block = 1;
            head_block = new Block();
            tail_block = head_block;
        }

        /**
         * clears the contents
         */
        void clear() {
            Block *tmp;
            while (head_block) {
                tmp = head_block;
                head_block = head_block->nxt_block;
                delete tmp;
            }
            tail_block = head_block = nullptr;
            num_block = 1;
            size_deque = 0;
        }

        /**
         * Deconstructor
         */
        ~deque() {
            clear();
        }

        Element *getElement(const int &index_deque, Block *&block, int &index_block) const {
            if (index_deque == size_deque) {
                index_block = tail_block->size_block;
                block = tail_block;
                return nullptr;
            }
            if (index_deque == size_deque - 1) {
                index_block = tail_block->size_block - 1;
                block = tail_block;
                return tail_block->tail_ele;
            }
            index_block = index_deque;
            block = head_block;
            while (index_block >= block->size_block) {
                index_block -= block->size_block;
                block = block->nxt_block;
            }
            return block->getEle(index_block);
        }

        Block *getBlock(const int &index_deque, int &index_block) const {
            if (!size_deque) {
                index_block = 0;
                return head_block;
            }
            if (index_deque == size_deque - 1) {
                index_block = tail_block->size_block - 1;
                return tail_block;
            }
            index_block = index_deque;
            Block *tmp = head_block;
            while (index_block >= tmp->size_block) {
                index_block -= tmp->size_block;
                tmp = tmp->nxt_block;
            }
            return tmp;
        }

        /**
         * assignment operator
         */
        deque &operator=(const deque &other) {
            if (this == &other) return *this;
            clear();
            Block *block = other.head_block;
            num_block = other.num_block;
            size_deque = other.size_deque;
            while (block) {
                if (!head_block) {
                    tail_block = new Block(*block);
                    head_block = tail_block;
                } else {
                    tail_block->nxt_block = new Block(*block);
                    tail_block->nxt_block->pre_block = tail_block;
                    tail_block = tail_block->nxt_block;
                }
                block = block->nxt_block;
            }
            return *this;
        }

        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T &at(const size_t &pos) {
            if (pos < 0 || pos >= size_deque) {
//                std::cout << "qwer625";///////todo
                throw index_out_of_bound();
            }
            Block *block;
            Element *ele;
            int ind;
            ele = getElement(pos, block, ind);
            return *(ele->data);
        }

        const T &at(const size_t &pos) const {
            if (pos < 0 || pos >= size_deque) {
//                std::cout << "qwer643";///////todo
                throw index_out_of_bound();
            }
            Block *block;
            Element *ele;
            int ind;
            ele = getElement(pos, block, ind);
            return *(ele->data);
        }

        T &operator[](const size_t &pos) {
            if (pos < 0 || pos >= size_deque) {
//                std::cout << "qwer655";///////todo
                throw index_out_of_bound();
            }
            Block *block;
            Element *ele;
            int ind;
            ele = getElement(pos, block, ind);
            return *(ele->data);
        }

        const T &operator[](const size_t &pos) const {
            if (pos < 0 || pos >= size_deque) {
//                std::cout << "qwer667";///////todo
                throw index_out_of_bound();
            }
            Block *block;
            Element *ele;
            int ind;
            ele = getElement(pos, block, ind);
            return *(ele->data);
        }

        /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
        const T &front() const {
            if (!size_deque) {
//                std::cout << "665smsp";//////////todo
                throw container_is_empty();
            }
            return *(head_block->head_ele->data);
        }

        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T &back() const {
            if (!size_deque) {
//                std::cout << "674smsp";//////////todo
                throw container_is_empty();
            }
            return *(tail_block->tail_ele->data);
        }

        /**
         * returns an iterator to the beginning.
         */
        iterator begin() {
            int index_deque = 0;
            return iterator(index_deque, this);
        }

        const_iterator cbegin() const {
            if (!size_deque) {
//                std::cout << "690smsp";//////////todo
                throw container_is_empty();
            }
            return const_iterator(0, this);
        }

        /**
         * returns an iterator to the end.
         */
        iterator end() {
            if (!size_deque) init();
            return iterator(size_deque, this);
        }

        const_iterator cend() const {
            return const_iterator(size_deque, this);
        }

        /**
         * checks whether the container is empty.
         */
        bool empty() const {
            if (size_deque) return false;
            return true;
        }

        /**
         * returns the number of elements
         */
        size_t size() const {
            return size_deque;
        }

        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert(iterator pos, const T &value) {
            if (pos.origin != this || !pos.isValid()) throw invalid_iterator();
            if (pos.ind_deque < 0 || pos.ind_block > size_deque) {
//                std::cout << "qwer758";///////todo
                throw index_out_of_bound();
            }
            if (!head_block) init();

            auto *ele = new Element(value);
            Block *block;
            int index_block;
            if (pos.ind_deque == size_deque) {
                if (tail_block->insert(tail_block->size_block, ele)) {
                    ++num_block;
                    tail_block = tail_block->nxt_block;
                }
            } else {
                block = getBlock(pos.ind_deque, index_block);
                if (block->insert(index_block, ele)) {
                    ++num_block;
                    if (block == tail_block) tail_block = block->nxt_block;
                }
            }
            ++size_deque;
            block = getBlock(pos.ind_deque, index_block);
            pos.ele = ele;
            pos.block = block;
            pos.ind_block = index_block;
            return pos;
        }

        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase(iterator pos) {
            if (pos.origin != this || !pos.isValid()) throw invalid_iterator();
            if (pos.ind_deque < 0 || pos.ind_deque >= size_deque) {
//                std::cout << "qwer795";///////todo
                throw index_out_of_bound();
            }
            if (!size_deque) {
//                std::cout << "761smsp";//////////todo
                throw container_is_empty();
            }
            int index_block;
            Block *block = getBlock(pos.ind_deque, index_block);
            bool flag = (block == tail_block), flag_ = (block->nxt_block == tail_block);
            Block *pre_block = block->pre_block;
            int merge_ = block->erase(index_block);
            if (merge_) {
                --num_block;
                if (merge_ == 1 && flag_) tail_block = block;
                if (merge_ == 2 && flag) tail_block = pre_block;
            }
            --size_deque;
            if (pos.ind_deque == size_deque) return end();
            Element *ele = getElement(pos.ind_deque, block, index_block);
            pos.ind_block = index_block;
            pos.block = block;
            pos.ele = ele;
            return pos;
        }

        /**
         * adds an element to the end
         */
        void push_back(const T &value) {
            insert(end(), value);
        }

        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back() {
            if (!size_deque) throw container_is_empty();
            erase(end() - 1);
        }

        /**
         * inserts an element to the beginning.
         */
        void push_front(const T &value) {
            insert(iterator(0, this), value);
        }

        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front() {
            if (!size_deque) {
                throw container_is_empty();
            }
            erase(begin());
        }

        void display() {
            int num = 0;
            for (Block *ptr = head_block; ptr; ptr = ptr->nxt_block) {
                num += ptr->size_block;
                ptr->display();
            }
            std::cout << num << std::endl;
        }

    };

}

#endif

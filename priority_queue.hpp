#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

    /**
     * a container like std::priority_queue which is a heap internal.
     */
    template<typename T, class Compare = std::less<T>>
    class priority_queue {
    private:
        struct Node {
            Node *left;
            Node *right;
            int dist; // 根结点到最近外界点距离 维护左偏堆
            T data;
            Node(T d,Node *l = nullptr,Node *r = nullptr,int Dist = 0):data(d),left(l),right(r),dist(Dist) {}
        };

        Node *root; //根结点
        int heapSize; //堆大小

        Node *buildHeap(Node *other) {
            if(other == nullptr) {
                return nullptr;
            }
            Node *r = new Node(other -> data,nullptr,nullptr,0);
            if(other -> left != nullptr) {
                r -> left = buildHeap(other -> left);
            }
            if(other -> right != nullptr) {
                r -> right = buildHeap(other -> right);
            }
            r -> dist = other -> dist;

            return r;
        }

        void clear(Node *r) {
            if(r == nullptr) {
                return ;
            }
            if(r -> left != nullptr) {
                clear(r -> left);
            }
            if(r -> right != nullptr) {
                clear(r -> right);
            }
            delete r;
        }

        Node *mergeHeap(Node *l,Node *r) {
            if(l == nullptr) {
                return r;
            }
            if(r == nullptr) {
                return l;
            }
            if(Compare()(l -> data,r -> data)) {
                std::swap(l ,r );
            }

            l -> right = mergeHeap(l -> right,r);
            if(l -> left == nullptr) {
                std::swap(l -> left, l -> right);
            }else if(l -> right != nullptr && l -> left -> dist < l -> right -> dist) {
                std::swap(l -> left, l -> right);
            }

            if(l -> right == nullptr) {
                l -> dist = 0;
            }else {
                l -> dist = l -> right -> dist + 1;
            }
            return l;
        }
    public:
        /**
         * TODO constructors
         */
        priority_queue() {
            root = nullptr;
            heapSize = 0;
        }
        priority_queue(const priority_queue &other) {
            heapSize = other.heapSize;
            root = buildHeap(other.root);
        }
        /**
         * TODO deconstructor
         */
        ~priority_queue() {
            clear(root);
        }
        /**
         * TODO Assignment operator
         */
        priority_queue &operator=(const priority_queue &other) {
            if(this == &other) {
                return *this;
            }
            clear(root);
            root = buildHeap(other.root);
            heapSize = other.heapSize;
            return *this;
        }
        /**
         * get the top of the queue.
         * @return a reference of the top element.
         * throw container_is_empty if empty() returns true;
         */
        const T & top() const {
            if(empty()) {
                throw container_is_empty();
            }
            return root -> data;
        }
        /**
         * TODO
         * push new element to the priority queue.
         */
        void push(const T &e) {
            Node *newElem = new Node(e,nullptr,nullptr,0);
            Node *oldRoot = root;
            try {
                root = mergeHeap(root,newElem);
                heapSize ++;
            }catch (...) {
                root = oldRoot;
                delete newElem;
                throw runtime_error();
            }
        }
        /**
         * TODO
         * delete the top element.
         * throw container_is_empty if empty() returns true;
         */
        void pop() {
            if(empty()) {
                throw container_is_empty();
            }
            Node *tmp = root;
            try {
                root = mergeHeap(root -> left,root -> right);
                heapSize --;
                delete tmp;//注意，这里需要把旧的根结点空间回收！
            }catch (...) {
                root = tmp;
                throw runtime_error();
            }

        }
        /**
         * return the number of the elements.
         */
        size_t size() const {
            return heapSize;
        }
        /**
         * check if the container has at least an element.
         * @return true if it is empty, false if it has at least an element.
         */
        bool empty() const {
            return heapSize == 0;
        }
        /**
         * merge two priority_queues with at most O(logn) complexity.
         * clear the other priority_queue.
         */
        void merge(priority_queue &other) {
            Node *oldroot = root;
            Node *otherOldRoot = other.root;
            try {
                root = mergeHeap(root,other.root);
                other.root = nullptr;
                heapSize += other.heapSize;
                other.heapSize = 0;
            }catch (...) {
                root = oldroot;
                other.root = otherOldRoot;
                throw runtime_error();
            }

        }
    };

}

#endif

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

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
	enum TreeType{LEFT,RIGHT};
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
    class iterator;
	class const_iterator;
private:
  	struct AVLnode {
  		value_type data;
  		AVLnode *left;
  		AVLnode *right;
  		AVLnode *parent;
  		int height; //结点高度

  		AVLnode(const value_type &element,AVLnode *l = nullptr,AVLnode *r = nullptr,AVLnode *p = nullptr,int h = 1):data(element),left(l),right(r),parent(p),height(h){}
  	};

	AVLnode *root;
	size_t Size;//结点个数

	AVLnode *buildTree(AVLnode *other,AVLnode *father = nullptr) { //深拷贝
		if(other == nullptr) {
			return nullptr;
		}
		AVLnode *newNode = new AVLnode(other -> data,nullptr,nullptr,nullptr,other -> height);
		if(other -> left != nullptr) {
			newNode -> left = buildTree(other -> left,newNode);
		}
		if(other -> right != nullptr) {
			newNode -> right = buildTree(other -> right,newNode);
		}
		newNode -> parent = father;
		return newNode;
	}

	void Clear(AVLnode *&t) { //析构
		if(t == nullptr) {
			return;
		}
		if(t -> left != nullptr) {
			Clear(t -> left);
		}
		if(t -> right != nullptr) {
			Clear(t -> right);
		}
		delete t;
	}

	//得到子树高度
	int Height(AVLnode *t) {
		if(t == nullptr) {
			return 0;
		}
		return t -> height;
	}

	//四种旋转方式 TODO 现在怀疑旋转时父结点没有正确更新 upd：现在正确了，基本可以认为插入没问题
	void LL(AVLnode *&t) {
		AVLnode *newRoot = t -> left;
		t -> left = newRoot -> right;
		newRoot -> right = t;
		newRoot -> parent = t -> parent;
		t -> parent = newRoot;
		if(t -> left != nullptr) {
			t -> left -> parent = t;
		} //TODO Add
		t -> height = std::max(Height(t -> left),Height(t -> right)) + 1;
		newRoot -> height = std::max(Height(t ),Height(newRoot -> left)) + 1;
		t = newRoot;
	}

	void RR(AVLnode *&t) {
		AVLnode *newRoot = t -> right;
		t -> right = newRoot -> left;
		newRoot -> left = t;
		newRoot -> parent = t -> parent;
		t -> parent = newRoot;
		if(t -> right != nullptr) {
			t -> right -> parent = t;
		} // TODO Add
		t -> height = std::max(Height(t -> left),Height(t -> right)) + 1;
		newRoot -> height = std::max(Height(t ),Height(newRoot -> right)) + 1;
		t = newRoot;
	}

	void LR(AVLnode *&t) {
		RR(t -> left);
		LL(t);
	}

	void RL(AVLnode *&t) {
		LL(t -> right);
		RR(t);
	}

	//find查找指定键值
	AVLnode *findKey(const Key& k) {
		AVLnode *r = root;
		while(r != nullptr) {
			if(Compare()(r->data.first ,k)) {
				//键值大于当前结点键值
				r = r -> right;
			}else if(Compare()(k,r->data.first)) {
				//键值小于当前结点键值
				r = r -> left;
			}else {
				break;
			}
		}
		return r;
	}

	const AVLnode *findKey(const Key &k) const {
		const AVLnode *r = root;
		while (r != nullptr) {
			if (Compare()(r->data.first, k)) {
				r = r->right;
			} else if (Compare()(k, r->data.first)) {
				r = r->left;
			} else {
				break;
			}
		}
		return r;
	}

	//insert插入操作
	AVLnode *insertElem(const value_type &elem,AVLnode *&t,AVLnode *&father,TreeType a,bool &isInsert) {
		AVLnode *pos = nullptr;
		if(t == nullptr) {
			t = new AVLnode(elem,nullptr,nullptr,father);
			if(father != nullptr) {
				if(a == LEFT) {
					father -> left = t;
				}else {
					father -> right = t;
				}
			}
			Size  ++;
			isInsert = true;
			pos = t;
		}else if(Compare()(t -> data.first,elem.first)) {
			//当前根结点小于插入元素，则插入右子树
			pos = insertElem(elem,t -> right,t,RIGHT,isInsert);
			//insertElem(elem,t -> right,t,RIGHT,isInsert);
			if(Height(t -> right) - Height(t -> left) == 2) { //失衡
				if(Compare()(t -> right -> data.first,elem.first)) {
					RR(t);
				}else {
					RL(t);
				}
			}
		}else if(Compare()(elem.first,t -> data.first)) {
			//插入左子树
			pos = insertElem(elem,t -> left,t,LEFT,isInsert);
			//insertElem(elem,t -> right,t,RIGHT,isInsert);
			if(Height(t -> left) - Height(t -> right) == 2) { //失衡
				if(Compare()(t -> left -> data.first,elem.first)) {
					LR(t);
				}else {
					LL(t);
				}
			}
		}else {
			isInsert = false;
			pos = t;
		}
		t -> height = std::max(Height(t -> left),Height(t -> right)) + 1;

		return pos;
	}

	//remove删除操作   TODO upd:remove操作仍然有问题 第一个测试点删除1结点时会出事，检查！ upd： 找到了1个子结点情形的错误
	bool removeElem(const Key &k,AVLnode *&t) {  //返回值表示树是否不会变矮 true表示树不会变矮，无需再调整，反之false
		if(t == nullptr) {
			return true;
		}
		if(!Compare()(t -> data.first,k) && !Compare()(k,t -> data.first)) { //要删除的就是根结点
			if(t -> left == nullptr && t -> right == nullptr) { //无子结点
				Size --;
				if(t -> parent != nullptr) {
					AVLnode *p = t -> parent;//这是必须的
					if(t -> parent -> left == t) {
						delete t;
						p -> left = nullptr;

						t = nullptr;
					}else {
						delete t;//顺序不能反 不然就相当于delete nullptr！！！
						p -> right = nullptr;//这里如果t -> parent -> right就会invalid read
						t = nullptr;
					}
					return false;
				}else {
					delete t;
					t = nullptr;
					return true;
				}
			}else if(t -> left == nullptr) {
				Size --;
				AVLnode *oldNode = t;
				//t = t -> right;
				AVLnode *f = t -> parent;
				t = t -> right;
				t -> parent = f;
				//注意！这里不可这样写是因为 oldNode -> parent也被修改了，已经发生了变化
				/*if(oldNode -> parent != nullptr) {
					if(oldNode -> parent -> left == oldNode) {
						oldNode -> parent -> left = t;
					}else {
						oldNode -> parent -> right = t;
					}
				}*/
				delete oldNode;
				oldNode = nullptr;
				return false;
			}else if(t -> right == nullptr) {
				Size --;
				AVLnode *oldNode = t;
				AVLnode *f = t -> parent;
				t = t -> left;
				t -> parent = f;
				//注意！这里不可这样写是因为 oldNode -> parent也被修改了，已经发生了变化
				/*if(oldNode -> parent != nullptr) {
					if(oldNode -> parent -> left == oldNode) {
						oldNode -> parent -> left = t;
					}else {
						oldNode -> parent -> right = t;
					}
				}*/
				delete oldNode;
				oldNode = nullptr;
				return false;
			}else { //依然拥有2个子结点
				//Size --;
				int H = Height(t);
				AVLnode *tmp = t -> right;
				bool flag = false;
				while(tmp -> left != nullptr) {
					tmp = tmp -> left;//找到最小结点作为新的根结点
					flag = true;
				}
				if(flag) {
					AVLnode *oldTmpRight = tmp -> right;
					if(tmp -> parent -> left == tmp) {
						tmp -> parent -> left = t;
					}else {
						tmp -> parent -> right = t;
					}
					AVLnode *oldTmpParent = tmp -> parent;
					tmp -> parent = t -> parent;
					t -> parent = oldTmpParent;
					tmp -> left = t -> left;
					tmp -> right = t -> right;
					t -> left -> parent = tmp;//t左右结点均存在
					t -> right -> parent = tmp;
					//至此tmp已经全部替换为t
					t -> right = oldTmpRight;
					t -> left  = nullptr;
					if(oldTmpRight != nullptr) {
						t -> right -> parent = t;
					}
				}else {
					AVLnode *oldTmp = tmp -> right;//TODO
					tmp -> parent = t -> parent;
					if(tmp -> right != nullptr) {
						tmp -> right -> parent = t;
					}
					tmp -> left = t -> left;
					tmp -> left -> parent = tmp;
					tmp -> right = t;
					t -> parent = tmp;
					t -> left = nullptr;
					t -> right = oldTmp ;
				}
				t -> height = tmp -> height;
				tmp -> height = H;
				//TODO
				//delete t;
				t = tmp;//TODO
				if(removeElem(k,t -> right)) {
					return true;
				}
				return adjust(t,RIGHT);//TODO
			}
		}else if(Compare()(t -> data.first,k)) {  //在右子树中找
			if(removeElem(k,t -> right)) {
				return true;
			}
			return adjust(t,RIGHT);
		}else {  //在左子树中找
			if(removeElem(k,t -> left)) {
				return true;
			}
			return adjust(t,LEFT);// 这里又出现了segmentation fault
		}
	}

	//调整平衡
	bool adjust(AVLnode *&t,TreeType a) {
		if(a == LEFT) { //在左子树上删除
			if(Height(t -> right) - Height(t -> left) == 1) {
				return true;
			}
			if(Height(t -> left) == Height(t -> right)) {
				t -> height --;
				return false;
			}
			if(Height(t -> right -> left) > Height(t -> right -> right)) {
				RL(t);
				return false;
			}
			RR(t);
			if(Height(t -> left) == Height(t -> right)) {  //TODO Segementation fault happens😢 upd: fixed
				return false;
			}
			return true;
		}else {  //在右子树上删除
			if(Height(t -> left) - Height(t -> right) == 1) {
				return true;
			}
			if(Height(t -> left) == Height(t -> right)) {
				t -> height --;
				return false;
			}
			if(Height(t -> left -> right) > Height(t -> left -> left)) {
				LR(t);
				return false;
			}
			LL(t);
			if(Height(t -> left ) == Height(t -> right)) { //逆天 这里笔误了
				return false;
			}
			return true;
		}
	}
public:
	class iterator {
		friend class map;
		friend class const_iterator;
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		map *AVL;//表示this map (vector理论上就该加的...
		AVLnode *p;
	public:
		iterator():AVL(nullptr),p(nullptr) {}
		iterator(const iterator &other):AVL(other.AVL),p(other.p) {}
		iterator(map *m,AVLnode *a):AVL(m),p(a){}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			iterator tmp = *this;
			++ (*this);
			return tmp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if(p == nullptr) {
				throw invalid_iterator();
			}
			if(p -> right != nullptr) {
				p = p -> right;
				while(p -> left != nullptr) {
					p = p -> left;
				}
			}else {
				AVLnode *father = p -> parent;
				while(father != nullptr && p == father -> right) {
					p = father;
					father = father -> parent;
				}
				p = father;
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			iterator tmp = *this;
			-- (*this);
			return tmp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if(p == nullptr) {
				p = AVL->root;
				if(p == nullptr) {
					throw invalid_iterator();
				}
				while(p -> right != nullptr) {
					p = p -> right;
				}
			}else if(p -> left != nullptr) {
				p = p -> left;
				while(p -> right != nullptr) {
					p = p -> right;
				}
			}else {
				AVLnode *father = p -> parent;
				while (father != nullptr && p == father -> left) {
					p = father;
					father = father -> parent;
				}
				p = father;
			}
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			if(p == nullptr) {
				throw invalid_iterator();
			}
			return p -> data;
		}
		bool operator==(const iterator &rhs) const {
			return AVL == rhs.AVL && p == rhs.p;
		}
		bool operator==(const const_iterator &rhs) const {
			return AVL == rhs.AVLconst && p == rhs.p_const;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			if(*this == rhs) {
				return false;
			}else {
				return true;
			}
		}
		bool operator !=(const const_iterator &rhs) const {
			if(*this == rhs) {
				return false;
			}else {
				return true;
			}
		}
		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			if(p == nullptr) {
				throw invalid_iterator();
			}
			return &(p -> data);
		}
	};
	class const_iterator {
		friend class map;
		friend class iterator;
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
		private:
			const map *AVLconst;
			const AVLnode *p_const;
		public:
			const_iterator():AVLconst(nullptr),p_const(nullptr) {}
			const_iterator(const const_iterator &other):AVLconst(other.AVLconst),p_const(other.p_const) {}
			const_iterator(const iterator &other):AVLconst(other.AVL),p_const(other.p){}
			// And other methods in iterator.
			// And other methods in iterator.
			// And other methods in iterator.
			const_iterator(const map *m,const AVLnode *a):AVLconst(m),p_const(a){}

			const_iterator operator++(int) {
				const_iterator temp = *this;
				++(*this);
				return temp;
			}

			const_iterator & operator++() {
				if (p_const == nullptr) {
					throw invalid_iterator();
				}
				if (p_const->right != nullptr) {
					p_const = p_const->right;
					while (p_const->left != nullptr) {
						p_const = p_const->left;
					}
				} else {
					const AVLnode *parent = p_const->parent;
					while (parent != nullptr && p_const == parent->right) {
						p_const = parent;
						parent = parent->parent;
					}
					p_const = parent;
				}
				return *this;
			}

			const_iterator operator--(int) {
				const_iterator temp = *this;
				--(*this);
				return temp;
			}

			const_iterator & operator--() {
				if (p_const == nullptr) {
					p_const = AVLconst->root;
					if (p_const == nullptr) {
						throw invalid_iterator();
					}
					while (p_const->right != nullptr) {
						p_const = p_const->right;
					}
				} else if (p_const->left != nullptr) {
					p_const = p_const->left;
					while (p_const->right != nullptr) {
						p_const = p_const->right;
					}
				} else {
					const AVLnode *parent = p_const->parent;
					while (parent != nullptr && p_const == parent->left) {
						p_const = parent;
						parent = parent->parent;
					}
					p_const = parent;
				}
				return *this;
			}

			bool operator==(const const_iterator &rhs) const {
				return AVLconst == rhs.AVLconst && p_const == rhs.p_const;
			}
			bool operator==(const iterator &rhs) const {
				return AVLconst == rhs.AVL && p_const == rhs.p;
			}

			bool operator!=(const const_iterator &rhs) const {
				if(*this == rhs) {
					return false;
				}else {
					return true;
				}
			}

			bool operator!=(const iterator &rhs) const {
				if(*this == rhs) {
					return false;
				}else {
					return true;
				}
			}

			const value_type & operator*() const {
				if (p_const == nullptr) {
					throw invalid_iterator();
				}
				return p_const->data;
			}

			const value_type* operator->() const noexcept {
				if(p_const == nullptr) {
					throw invalid_iterator();
				}
				return &(p_const -> data);
			}
	};
	/**
	 * TODO two constructors
	 */
	map() {
		root = nullptr;
		Size = 0;
	}
	map(const map &other) {
		if(other.root == nullptr) {
			root = nullptr;
			Size = 0;
		}else {
			root = buildTree(other.root,nullptr);
			Size = other.Size;
		}
	}
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
		if(this == &other) {
			return *this;
		}
		clear();
		if(other.root != nullptr) {
			root = buildTree(other.root,nullptr);
			Size = other.Size;
		}else {
			root = nullptr;
			Size = 0;
		}
		return *this;
	}
	/**
	 * TODO Destructors
	 */
	~map() {
		Clear(root);
		root = nullptr;
		Size = 0;
	}
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		AVLnode *target = findKey(key);
		if(target == nullptr) {
			throw index_out_of_bound();
		}
		return target -> data.second;
	}
	const T & at(const Key &key) const {
		const AVLnode *target = findKey(key);
		if(target == nullptr) {
			throw index_out_of_bound();
		}
		return target -> data.second;
	}
	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		AVLnode *target = findKey(key);
		if(target == nullptr) {
			value_type Pair = {key,T()};
			bool isinsert;
			AVLnode *father = nullptr;
			target = insertElem(Pair,root,father,LEFT,isinsert);
		}
		return target->data.second;
	}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		return at(key);
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		AVLnode *p = root;
		if(p != nullptr) {
			while(p -> left != nullptr) {
				p = p -> left;
			}
		}
		return iterator(this,p);
	}
	const_iterator cbegin() const {
		const AVLnode *p = root;
		if(p != nullptr) {
			while(p -> left != nullptr) {
				p = p -> left;
			}
		}
		return const_iterator(this,p);
	}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(this,nullptr);
	}
	const_iterator cend() const {
		return const_iterator(this,nullptr);
	}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return root == nullptr;
	}
	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return Size;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		Clear(root);
		root = nullptr;
		Size = 0;
	}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) { //TODO 这里在引用指针上出现了问题
		bool isinsert;
		AVLnode *target ;
		AVLnode *father = nullptr;
		target = insertElem(value,root,father,LEFT,isinsert);
		/*if (!isinsert) {
			delete target; // 释放临时对象
			target = nullptr;
		}*/
		pair<iterator,bool> Pair = {iterator(this,target),isinsert};
		return Pair;
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if(pos.AVL != this) {
			throw invalid_iterator();
		}
		if(pos.p == nullptr) {
			throw invalid_iterator();
		}
		removeElem(pos.p -> data.first,root);
	}
	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {
		const AVLnode *p = findKey(key);
		if(p == nullptr) {
			return 0;
		}else {
			return 1;
		}
	}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		AVLnode *p = findKey(key);
		return iterator(this,p);
	}
	const_iterator find(const Key &key) const {
		const AVLnode *p = findKey(key);
		return const_iterator(this,p);
	}
};

}

#endif
template<class T>
class BinSearchTreeNode
{
public:
	T key;
	BinSearchTreeNode<T>* parent;
	BinSearchTreeNode<T>* lchild;
	BinSearchTreeNode<T>* rchild;
};

template<class T, class Fun>
class BinSearchTree
{
public:
	BinSearchTreeNode<T>* root;

public:
	BinSearchTree() : root(NULL) {}

	BinSearchTreeNode<T>* bin_search(BinSearchTreeNode<T> root, T key) //递归形式
	{
		if(NULL == root || key == root->key) {
			return root;
		}
		else if(key < root->key) {
			return search_cur(root->lchild);
		}

		return search_cur(root->rchild);
	}

	BinSearchTreeNode<T>* bin_search(T key) //非递归形式
	{
		BinSearchTreeNode<T>* p = this->root;
		while(NULL != p) {
			if(key == p->key) return p;
			if(key < p->key) p = p->lchild;
			else p = p->rchild;
		}
		return p;
	}

	BinSearchTreeNode<T>* bin_insert(T key)
	{
		BinSearchTreeNode<T>* ins_node = new BinSearchTreeNode<T>;
		ins_node->key = key;
		ins_node->lchild = ins_node->rchild = NULL;

		if(NULL == root) {
			root = ins_node;
			root->parent = NULL;
			return ins_node;
		}

		BinSearchTreeNode<T>* p = this->root;
		BinSearchTreeNode<T>* q = p;
		do {
			p = q;
			if(key < p->key) q = p->lchild;
			else q = p->rchild;
		} while(NULL != q);

		if(key < p->key) {
			p->lchild = ins_node;
			ins_node->parent = p;
		}
		else {
			p->rchild = ins_node;
			ins_node->parent = p;
		}
		return ins_node;
	}

	void bin_delete(T key)
	{
		BinSearchTreeNode<T>* del_node = bin_search(key);

		if(NULL == del_node->rchild || NULL == del_node->lchild) {
			BinSearchTreeNode<T>* child = NULL;
			if(NULL != del_node->rchild) child = del_node->rchild;
			else if(NULL != del_node->lchild) child = del_node->lchild;
			if(del_node->key < del_node->parent->key) del_node->parent->lchild = child;
			else del_node->parent->rchild = child;
			if(NULL != child) child->parent = del_node->parent;
			delete del_node;
			del_node = NULL;
			return;
		}

		BinSearchTreeNode<T>* successor = bin_successor(del_node);
		del_node ->key = successor->key;
		this->bin_delete(successor);
	}

	void bin_delete(BinSearchTreeNode<T>* del_node)
	{
		if(NULL == del_node->rchild || NULL == del_node->lchild) {
			BinSearchTreeNode<T>* child = NULL;
			if(NULL != del_node->rchild) child = del_node->rchild;
			else if(NULL != del_node->lchild) child = del_node->lchild;
			if(del_node->key < del_node->parent->key) del_node->parent->lchild = child;
			else del_node->parent->rchild = child;
			if(NULL != child) child->parent = del_node ->parent;
			delete del_node;
			del_node = NULL;

		}

		BinSearchTreeNode<T>* successor = bin_successor(del_node);
		del_node ->key = successor->key;
		this->bin_delete(successor);
	}

	void inorder_traverse(BinSearchTreeNode<T>* root, Fun fun)
	{	
		if(NULL != root) {
			inorder_traverse(root->lchild, fun);
			fun(root);
			inorder_traverse(root->rchild, fun);
		}
		// return NULL;
	}

private:
	BinSearchTreeNode<T>* min_node(BinSearchTreeNode<T>* root)
	{
		BinSearchTreeNode<T>* p = root;
		while(NULL != p->lchild) {
			p = p->lchild;
		}
		return p;
	}

	BinSearchTreeNode<T>* max_node(BinSearchTreeNode<T>* root)
	{
		BinSearchTreeNode<T>* p = root;
		while(NULL != p->rchild) {
			p = p->rchild;
		}
		return p;
	}


	/*
	* 若根节点的右子树存在，直接后继为右子树的最小节点；
	* 若根节点的右子树不存在，直接后继为位于左分支上的最近双亲节点
	*/
	BinSearchTreeNode<T>* bin_successor(BinSearchTreeNode<T>* node)
	{
		if(NULL != node->rchild) return min_node(node->rchild);

		BinSearchTreeNode<T>* p = node;
		BinSearchTreeNode<T>* q = node->parent;
		while(NULL != q) { //上溯，寻找位于左分支上的最近双亲节点
			if(p == q->lchild) return q;
			q = q->parent;
			p = q;
		}
	}

	/*
	* 若根节点的左子树存在，直接后继为左子树的最大节点；
	* 若根节点的左子树不存在，直接后继为位于右分支上的最近双亲节点
	*/
	BinSearchTreeNode<T>* pre_successor(BinSearchTreeNode<T>* node)
	{
		if(NULL != node->lchild) return max_node(node->lchild);

		BinSearchTreeNode<T>* p = node;
		BinSearchTreeNode<T>* q = node->parent;
		while(NULL != q) { //上溯，寻找位于左分支上的最近双亲节点
			if(p == q->rchild) return q;
			q = q->parent;
			p = q;
		}
	}

}; //BinSearchTree
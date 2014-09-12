/*
* 二叉排序树实现
*/

#ifndef __BIN_SEARCH_TREE_H__
#define __BIN_SEARCH_TREE_H__

#include <stack>

using namespace std;

template<class T>
class BinSearchTreeNode
{
public:
	T key;
	bool flag; //用于后序非递归遍历
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

	//非递归后序遍历销毁每个节点
	~BinSearchTree()
	{
		stack<BinSearchTreeNode<T>*> s;
		BinSearchTreeNode<T>* p = root;

		while(NULL != p || false == s.empty()) {
			while(NULL != p) { //遍历左子树
				p->flag = true; //标记为第一次访问
				s.push(p);
				p = p->lchild;
			}

			if(false == s.empty()) { //遍历右子树
				p = s.top();
				if(true == p->flag) { //标记为第二次访问
					p->flag = false; 
					p = p->rchild;
				}
				else { //第三次访问
					delete p;
					s.pop();
					p = NULL;
				}
			}
		} //while
	}

	BinSearchTreeNode<T>* find_node(T key) //非递归形式
	{
		BinSearchTreeNode<T>* p = this->root;
		while(NULL != p) {
			if(key == p->key) return p;
			if(key < p->key) p = p->lchild;
			else p = p->rchild;
		}

		return p;
	}

	//若树中存在相同的key，则新节点插在相同key节点的右子树上
	BinSearchTreeNode<T>* insert_node(T key)
	{
		BinSearchTreeNode<T>* ins_node = new BinSearchTreeNode<T>;
		ins_node->key = key;
		ins_node->lchild = ins_node->rchild = NULL;

		/*空树情形*/
		if(NULL == root) {
			root = ins_node;
			root->parent = NULL;
			return ins_node;
		}

		/*不为空树的情形*/
		BinSearchTreeNode<T>* p = this->root;
		BinSearchTreeNode<T>* q = p;
		do {
			p = q;
			if(key < p->key) q = p->lchild;
			else q = p->rchild;
		} while(NULL != q); //直到q为空，停止循环

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

	void delete_node(BinSearchTreeNode<T>* node)
	{
		if(NULL == node) return;

		/*被删除节点最多只有一个子节点的情形*/
		if(NULL == node->rchild || NULL == node->lchild) {
			//找到被删除节点的子节点,可能为空
			BinSearchTreeNode<T>* child = NULL;
			if(NULL != node->rchild) child = node->rchild;
			else if(NULL != node->lchild) child = node->lchild;

			//将被删除节点的子节点成为被删除节点的父节点的子节点
			if(node->key < node->parent->key) node->parent->lchild = child;
			else node->parent->rchild = child;
			if(NULL != child) child->parent = node->parent;

			//释放空间
			delete node;
			node = NULL;

			return; //递归结束
		}

		/*被删除节点有两个子节点的情形*/
		BinSearchTreeNode<T>* successor = bin_successor(node); //找到被删除节点的后继节点
		node->key = successor->key;
		this->delete_node(successor); //转化为删除后继节点
	}

	//非递归先序遍历
	void preorder_traverse(Fun fun)
	{
		stack<BinSearchTreeNode<T>*> s;
		BinSearchTreeNode<T>* p = root;

		while(NULL != p || false == s.empty()) {
			while(NULL != p) {
				fun(p);
				s.push(p);
				p = p->lchild;
			}

			if(false == s.empty()) {
				p = s.top();
				s.pop();
				p = p->rchild;
			}
		}

		return;
	}

	//非递归中序遍历
	void inorder_traverse(Fun fun)
	{
		stack<BinSearchTreeNode<T>*> s;
		BinSearchTreeNode<T>* p = root;

		while(NULL != p || false == s.empty()) {
			while(NULL != p) {
				s.push(p);
				p = p->lchild;
			}

			if(false == s.empty()) {
				p = s.top();
				fun(p);
				s.pop();
				p = p->rchild;
			}
		}
		
		return;
	}

	//非递归后序遍历
	void postorder_traverse(Fun fun)
	{
		stack<BinSearchTreeNode<T>*> s;
		BinSearchTreeNode<T>* p = root;

		while(NULL != p || false == s.empty()) {
			while(NULL != p) { //遍历左子树
				p->flag = true; //标记为第一次访问
				s.push(p);
				p = p->lchild;
			}

			if(false == s.empty()) { //遍历右子树
				p = s.top();
				if(true == p->flag) { //标记为第二次访问
					p->flag = false; 
					p = p->rchild;
				}
				else { //第三次访问
					fun(p);
					s.pop();
					p = NULL;
				}
			}
		} //while
		
		return;
	}

	//递归先序遍历
	void preorder_traverse(BinSearchTreeNode<T>* root, Fun fun)
	{	
		if(NULL != root) {
			fun(root);
			preorder_traverse(root->lchild, fun);
			preorder_traverse(root->rchild, fun);
		}

		return;
	}

	//递归中序遍历
	void inorder_traverse(BinSearchTreeNode<T>* root, Fun fun)
	{	
		if(NULL != root) {
			inorder_traverse(root->lchild, fun);
			fun(root);
			inorder_traverse(root->rchild, fun);
		}

		return;
	}

	//递归后序遍历
	void postorder_traverse(BinSearchTreeNode<T>* root, Fun fun)
	{	
		if(NULL != root) {
			postorder_traverse(root->lchild, fun);
			postorder_traverse(root->rchild, fun);
			fun(root);
		}

		return;
	}

protected:
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

#endif

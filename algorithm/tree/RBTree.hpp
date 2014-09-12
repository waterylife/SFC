/*
* 红黑树实现
* 初始版本：20140615
*/

#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include "common.h"
#include <iostream>

using namespace std;

template <class T>
class RBTreeNode
{
public:
	T key;
	unsigned int color; //0: red, 1: black
	RBTreeNode<T>* parent;
	RBTreeNode<T>* lchild;
	RBTreeNode<T>* rchild;
};

template <class T, class VisitFun>
class RBTree
{
public:
	RBTreeNode<T>* root;
	RBTreeNode<T>* leaf;

public:
	RBTree()
	{
		leaf = new RBTreeNode<T>;
		leaf->color = 1;
		leaf->parent = leaf->lchild = leaf->rchild = NULL;

		root = leaf;
	}

	~RBTree()
	{
		while(leaf != root) {
			rb_delete(root);
		}
		delete leaf; leaf = NULL;
	}

	RBTreeNode<T>* bin_search(RBTreeNode<T> root, T key) //递归形式
	{
		if(NULL == root || key == root->key) {
			return root;
		}
		else if(key < root->key) {
			return search_cur(root->lchild);
		}

		return search_cur(root->rchild);
	}

	RBTreeNode<T>* bin_search(T key) //非递归形式
	{
		RBTreeNode<T>* p = this->root;
		while(NULL != p) {
			if(key == p->key) return p;
			if(key < p->key) p = p->lchild;
			else p = p->rchild;
		}
		return p;
	}

	RBTreeNode<T>* rb_insert(T key)
	{
		RBTreeNode<T>* ins_node = new RBTreeNode<T>;
		ins_node->key = key;
		ins_node->lchild = ins_node->rchild = leaf;

		if(leaf == root) {
			root = ins_node;
			root->parent = leaf;
			root->color = 1;
			return ins_node;
		}

		RBTreeNode<T>* p = this->root;
		RBTreeNode<T>* q = p;
		do {
			p = q;
			if(key < p->key) q = p->lchild;
			else q = p->rchild;
		} while(leaf != q);

		if(key < p->key)	p->lchild = ins_node;
		else	p->rchild = ins_node;
		ins_node->parent = p;
		ins_node->color = 0;
		rb_insert_fixup(ins_node);
		return ins_node;
	}

	void rb_delete(T key)
	{
		RBTreeNode<T>* del_node = bin_search(key);

		rb_delete(del_node);
	}

	void rb_delete(RBTreeNode<T>* del_node)
	{
		if(leaf == del_node->rchild || leaf == del_node->lchild) {
			RBTreeNode<T>* child = leaf;
			if(leaf != del_node->rchild) child = del_node->rchild;
			else if(leaf != del_node->lchild) child = del_node->lchild;
			if(del_node->key < del_node->parent->key) del_node->parent->lchild = child;
			else del_node->parent->rchild = child;
			child->parent = del_node ->parent;
			
			if(root == del_node) root = child;
			if(1 == del_node->color) rb_delete_fixup(child);

			delete del_node;
			del_node = NULL;
		}
		else {
			RBTreeNode<T>* successor = bin_successor(del_node);
			del_node ->key = successor->key;
			this->rb_delete(successor);
		}
	}

	void inorder_traverse(RBTreeNode<T>* root, VisitFun fun)
	{	
		if(leaf != root) {
			inorder_traverse(root->lchild, fun);
			fun(root);
			inorder_traverse(root->rchild, fun);
		}
	}

	void preorder_traverse(RBTreeNode<T>* root, VisitFun fun)
	{	
		if(leaf != root) {
			fun(root);
			preorder_traverse(root->lchild, fun);
			preorder_traverse(root->rchild, fun);
		}
	}

private:
	void left_rotate(RBTreeNode<T>* node)
	{
		if(leaf == node->rchild) return;
		
		RBTreeNode<T>* p = node->rchild;
		if(node == node->parent->lchild) node->parent->lchild = p;
		else node->parent->rchild = p;
		p->parent = node->parent;
		node->parent = p;
		node->rchild = p->lchild;
		p->lchild->parent = node;
		p->lchild = node;

		if(root == node) root = p;
	}

	void right_rotate(RBTreeNode<T>* node)
	{
		if(leaf == node->lchild) return;

		RBTreeNode<T>* p = node->lchild;
		if(node == node->parent->lchild) node->parent->lchild = p;
		else node->parent->rchild = p;
		p->parent = node->parent;
		node->parent = p;
		node->lchild = p->rchild;
		p->rchild->parent = node;
		p->rchild = node;

		if(root == node) root = p;
	}

	void rb_insert_fixup(RBTreeNode<T>* ins_node)
	{
		RBTreeNode<T>* p = leaf;
		while(0 == (p = ins_node->parent)->color) { //p必定不是根节点，其父节点存在
			if(p == p->parent->lchild) {
				RBTreeNode<T>* q = p->parent->rchild;
				if(leaf != q && 0 == q->color) {
					p->color = 1;
					q->color = 1;
					ins_node = p->parent; 
					ins_node->color = 0;
					continue;
				}
				if(ins_node == p->rchild) {
					left_rotate(p);
					sfc_swap(p, ins_node);
				}
				p->color = 1;
				p->parent->color = 0;
				right_rotate(p->parent);
			}
			else { //算法与上一个分支对称
				RBTreeNode<T>* q = p->parent->lchild;
				if(leaf != q && 0 == q->color) {
					p->color = 1;
					q->color = 1;
					ins_node = p->parent;
					ins_node->color = 0;
					continue;
				}
				if(ins_node == p->lchild) {
					right_rotate(p);
					sfc_swap(p, ins_node);
				}
				p->color = 1;
				p->parent->color = 0;
				left_rotate(p->parent);
			}
		} //while
		root->color = 1;
	}

	void rb_delete_fixup(RBTreeNode<T>* child)
	{
		(child->color) ++; //增加一层黑色属性
		while(child != root && child->color == 2) {
			RBTreeNode<T>* p = child->parent;
			if(child == p->lchild) {
				RBTreeNode<T>* q = p->rchild;
				if(0 == q->color) { //case 1
					q->color = 1;
					p->color = 0;
					left_rotate(p);
					q = p->rchild;
				}
				if(1 <= q->lchild->color && 1 <= q->rchild->color) { //case 2
					(child->color) --;
					(q->color) --;
					(p->color) ++;
					child = p;
					continue;
				}
				if(1 == q->rchild->color) { //case 3
					q->color = 0;
					q->lchild->color = 1;
					right_rotate(q);
					q = q->lchild;
				}
				//case 4
				sfc_swap(p->color, q->color);
				q->rchild->color = 1;
				left_rotate(p);
				(child->color) --;
			}
			else { //算法与上一个分支对称
				RBTreeNode<T>* q = p->lchild;
				if(0 == q->color) { //case 1
					q->color = 1;
					p->color = 0;
					right_rotate(p);
					q = p->lchild;
				}
				if(1 <= q->lchild->color && 1 <= q->rchild->color) { //case 2
					(child->color) --;
					(q->color) --;
					(p->color) ++;
					child = p;
					continue;
				}
				if(1 == q->lchild->color) { //case 3
					q->color = 0;
					q->rchild->color = 1;
					left_rotate(q);
					q = q->rchild;
				}
				//case 4
				sfc_swap(p->color, q->color);
				q->lchild->color = 1;
				right_rotate(p);
				(child->color) --;
			}
		}
		root->color = 1;
	}

	RBTreeNode<T>* min_node(RBTreeNode<T>* root)
	{
		RBTreeNode<T>* p = root;
		while(leaf != p->lchild) {
			p = p->lchild;
		}
		return p;
	}

	RBTreeNode<T>* max_node(RBTreeNode<T>* root)
	{
		RBTreeNode<T>* p = root;
		while(leaf != p->rchild) {
			p = p->rchild;
		}
		return p;
	}


	/*
	* 若根节点的右子树存在，直接后继为右子树的最小节点；
	* 若根节点的右子树不存在，直接后继为位于左分支上的最近双亲节点
	*/
	RBTreeNode<T>* bin_successor(RBTreeNode<T>* node)
	{
		if(leaf != node->rchild) return min_node(node->rchild);

		RBTreeNode<T>* p = node;
		RBTreeNode<T>* q = node->parent;
		while(leaf != q) { //上溯，寻找位于左分支上的最近双亲节点
			if(p == q->lchild) return q;
			q = q->parent;
			p = q;
		}
	}

	/*
	* 若根节点的左子树存在，直接后继为左子树的最大节点；
	* 若根节点的左子树不存在，直接后继为位于右分支上的最近双亲节点
	*/
	RBTreeNode<T>* pre_successor(RBTreeNode<T>* node)
	{
		if(leaf != node->lchild) return max_node(node->lchild);

		RBTreeNode<T>* p = node;
		RBTreeNode<T>* q = node->parent;
		while(leaf != q) { //上溯，寻找位于左分支上的最近双亲节点
			if(p == q->rchild) return q;
			q = q->parent;
			p = q;
		}
	}
};

#endif

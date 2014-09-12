/*
* AVL树实现
*/
#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include "BinSearchTree.hpp"

template<class T>
class AVLTreeNode : public BinSearchTreeNode<T>
{
public:
	int height; //一棵空树的高度为-1，只有一个根节点的树的高度为0，
			    //以后每多一层高度加1。
};

template<class T, class Fun>
class AVLTree : public BinSearchTree<T, Fun>
{
private:
	void single_rotate_right(BinSearchTreeNode<T>* node)
	{
		BinSearchTreeNode<T>* x = node->lchild;
		BinSearchTreeNode<T>* y = x->rchild;

		node-lchild = y;
		y->parent = node;

		x->rchild = node;
		node->parent->lchild = x;
		x->parent = node->parent;
		node->parent = x;

		node->height = ((node->lchild->height > node->rchild->height) ? node-lchild->height : node->rchild->height) + 1;
		x->height = ((x->lchild->height > x->rchild->height) ? x-lchild->height : x->rchild->height) + 1;
	}

	void single_rotate_left(BinSearchTreeNode<T>* node)
	{
		BinSearchTreeNode<T>* x = node->rchild;
		BinSearchTreeNode<T>* y = x->lchild;

		node->rchild = y;
		y->parent = node;

		x->lchild = node;
		node->parent->rchild = x;
		x->parent = node->parent;
		node->parent = x;

		node->height = ((node->lchild->height > node->rchild->height) ? node-lchild->height : node->rchild->height) + 1;
		x->height = ((x->lchild->height > x->rchild->height) ? x-lchild->height : x->rchild->height) + 1;
	}

	void double_rotate_lr(BinSearchTreeNode<T>* node)
	{
		BinSearchTreeNode<T>* x = node->rchild;

		single_rotate_left(node);
		single_rotate_right(x);
	}

	void double_rotate_rl(BinSearchTreeNode<T>* node)
	{
		BinSearchTreeNode<T>* x = node->lchild;

		single_rotate_right(node);
		single_rotate_left(x);
	}
	
};

#endif
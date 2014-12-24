#include "AVLTree.h"


template<class T, class Fun>
void AVLTree<T, Fun>::single_rotate_right(BinSearchTreeNode<T>* node)
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

template<class T, class Fun>
void AVLTree<T>::single_rotate_left(BinSearchTreeNode<T>* node)
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

template<class T, class Fun>
void AVLTree<T>::double_rotate_lr(BinSearchTreeNode<T>* node)
{
	BinSearchTreeNode<T>* x = node->rchild;

	single_rotate_left(node);
	single_rotate_right(x);
}

template<class T, class Fun>
void AVLTree<T>::double_rotate_rl(BinSearchTreeNode<T>* node)
{
	BinSearchTreeNode<T>* x = node->lchild;

	single_rotate_right(node);
	single_rotate_left(x);
}
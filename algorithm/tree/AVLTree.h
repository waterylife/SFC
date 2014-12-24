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
	void single_rotate_right(BinSearchTreeNode<T>* node);
	void single_rotate_left(BinSearchTreeNode<T>* node);
	void double_rotate_lr(BinSearchTreeNode<T>* node);
	void double_rotate_rl(BinSearchTreeNode<T>* node);
};

#endif //__AVL_TREE_H__
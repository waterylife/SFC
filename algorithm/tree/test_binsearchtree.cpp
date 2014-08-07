#include <iostream>

#include "BinSearchTree.hpp"

using namespace std;

typedef void (*EchoNode)(BinSearchTreeNode<int>* node);

void echo_node(BinSearchTreeNode<int>* node)
{
	cout<<node->key<<"\t";
}

int main()
{
	BinSearchTree<int, EchoNode> tree;

	//构造二叉查找树
	tree.bin_insert(50);
	tree.bin_insert(31);
	tree.bin_insert(66);
	tree.bin_insert(20);
	tree.bin_insert(32);
	tree.bin_insert(9);
	tree.bin_insert(25);
	tree.bin_insert(21);
	tree.bin_insert(27);
	tree.bin_insert(49);


	cout<<"Result of inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;

	tree.bin_delete(32);
	cout<<"Result of inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;

	return 0;
}
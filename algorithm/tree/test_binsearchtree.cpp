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
	tree.insert_node(50);
	tree.insert_node(31);
	tree.insert_node(66);
	tree.insert_node(20);
	tree.insert_node(32);
	tree.insert_node(9);
	tree.insert_node(25);
	tree.insert_node(21);
	tree.insert_node(27);
	tree.insert_node(49);

	BinSearchTreeNode<int>* node = tree.find_node(9);
	if(NULL == node) {
		cout<<"node has not been found"<<endl;
		return 0;
	}

	cout<<"Result of cur preorder traverse:"<<endl;
	tree.preorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of noncur preorder traverse:"<<endl;
	tree.preorder_traverse(echo_node);
	cout<<endl;

	cout<<"Result of cur inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of noncur inorder traverse:"<<endl;
	tree.inorder_traverse(echo_node);
	cout<<endl;

	cout<<"Result of cur postorder traverse:"<<endl;
	tree.postorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of noncur postorder traverse:"<<endl;
	tree.postorder_traverse(echo_node);
	cout<<endl;


	tree.delete_node(node);
	cout<<"Result of inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;

	return 0;
}

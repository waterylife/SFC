#include <iostream>

#include "RBTree.hpp"

using namespace std;

typedef void (*EchoNode)(RBTreeNode<int>* node);

void echo_node(RBTreeNode<int>* node)
{
	cout<<node->key<<","<<node->color<<"\t";
}

int main()
{
	RBTree<int, EchoNode> tree;

	//构造二叉查找树
	tree.rb_insert(50);
	tree.rb_insert(31);
	tree.rb_insert(66);
	tree.rb_insert(20);
	tree.rb_insert(32);
	tree.rb_insert(9);
	tree.rb_insert(25);
	tree.rb_insert(21);
	tree.rb_insert(27);
	tree.rb_insert(49);


	cout<<"Result of inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of preorder traverse: "<<endl;
	tree.preorder_traverse(tree.root, echo_node);
	cout<<endl;

	tree.rb_delete(32);
	/*cout<<"Result of inorder traverse after delete 32:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;*/

	tree.rb_delete(50);
	cout<<"Result of inorder traverse after delete:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of preorder traverse: "<<endl;
	tree.preorder_traverse(tree.root, echo_node);
	cout<<endl;

	/*tree.rb_delete(31);
	tree.rb_delete(49);
	tree.rb_delete(20);
	tree.rb_delete(66);
	cout<<"Result of inorder traverse: "<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;
	cout<<"Result of preorder traverse: "<<endl;
	tree.preorder_traverse(tree.root, echo_node);
	cout<<endl;*/


/*	tree.rb_delete(31);
	cout<<"Result of inorder traverse:"<<endl;
	tree.inorder_traverse(tree.root, echo_node);
	cout<<endl;*/

	return 0;
}
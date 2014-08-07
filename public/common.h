#ifndef __COMMON_H
#define __COMMON_H

#include <iostream>

using namespace std;

extern bool ascend_comp(int left, int right);
extern bool descend_comp(int left, int right);

template<class T>
void sfc_swap(T& left, T& right)
{
	/*if(left != right) {
		left ^= right;
		right ^= left;
		left ^= right;
	}*/
	T tmp;
	tmp = left;
	left = right;
	right = tmp;
}

template <class T>
void echo_array(T* p_array, int size)
{
	T* p_tmp = p_array;

	for(int i = 0; i < size; i++) {
		cout<<*p_tmp<<"\t";
		p_tmp ++;
	}
	cout<<endl;
}


#endif

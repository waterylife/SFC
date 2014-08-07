#include <boost/smart_ptr.hpp>
#include <iostream>

using namespace boost;
using namespace std;

/*
* 直接插入排序
* 复杂度：O(n2)
*/
template <class T, class commpFun>
int insert_sort(shared_ptr<T> sp, int size, commpFun commp)
{
	for(int i = 1; i < size; i++) {
		for(int j = 0; j < i; j++) {  //顺序查找
			if(commp(*(sp.get() + i), *(sp.get() + j))) {
				T tmp = *(sp.get() + i);
				int k = i;
				while(k > j) {
					*(sp.get() + k) = *(sp.get() + k -1);
					k--;
				}
				*(sp.get() + j) = tmp;
				break;
			}
		}
	}

	return 0;
}

template <class T, class commpFun>
int insert_sort2(T *ptr, int size, commpFun commp)
{
	for(int i = 1; i < size; i++) {
		if(commp(ptr[i], ptr[i-1])) { //需插入
			T tmp = ptr[i];
			ptr[i] = ptr[i - 1];
			int j = i -1;
			for(; j > 0 && commp(tmp, ptr[j - 1]); j--) { //寻找i节点元素应当插入的位置
				ptr[j] = ptr[j -1];
			}
			ptr[j] = tmp;
		} //if
	}

	return 0;
}

/*
* 直接选择排序
*/
template<class T, class commpFun>
int select_sort(T* ptr, int size, commpFun commp)
{
	for(int i = 0; i < size; i++) {
		int min_pos = i;
		for(int j = i + 1; j < size; j++) { //从无序区（i+1到size-1）中选择出一个最下的元素
			if(commp(ptr[j], ptr[min_pos])) min_pos = j;
		}
		my_swap(ptr[i], ptr[min_pos]); //将无序区（i+1到size-1）中的最小元素与i交换，扩大有序区
	}

	return 0;
}

/*
* 希尔排序
*/
template<class T, class commpFun>
int shell_sort(T* ptr, int size, commpFun commp)
{
	for(int gap = size / 2; gap > 0; gap /= 2) { //分组，一共gap个分组，循环，直到只有一个分组为止
		for(int i = 0; i < gap; i++) { //分别对每一组进行直接插入排序
			for(int j = i + gap; j < size; j += gap) { //单组直接插入排序
				if(commp(ptr[j], ptr[j-gap])) {
					swap(ptr[j], ptr[j-gap]);
					int k = j - gap;
					for(; k > 0 && commp(ptr[k], ptr[k-gap]); k -= gap) {
						my_swap(ptr[k], ptr[k-gap]);
					}
				}
			} //j
		} //i
	} //gap

	return 0;
}

/*
* 归并排序
*/
template<class T, class commpFun>
int mergeArray(T* ptr, int first, int last, int mid, commpFun commp)
{
	int i = first; int j = mid + 1; int k = 0;
	T tmp[last - first + 1]; //额外申请空间用于存放排序结果

	while(i <= mid && j <= last) {
		if(commp(ptr[i], ptr[j])) {
			tmp[k++] = ptr[i++];
		}
		else {
			tmp[k++] = ptr[j++];
		}
	}

	while(i <= mid) {
		tmp[k++] = ptr[i++];
	}
	while(j <= last) {
		tmp[k++] = ptr[j++];
	}

	for(i = 0; i < k; i++) {
		ptr[first + i] = tmp[i];
	}

	return 0;
}

template<class T, class commpFun>
int mergeSort(T* ptr, int first, int last, commpFun commp)
{
	int mid = (first + last) / 2;
	if(first < last) { //队列中不止一个元素
		mergeSort(ptr, first, mid, commp);
		mergeSort(ptr, mid + 1, last, commp);
		mergeArray(ptr, first, last, mid, commp);
	}

	return 0;
}

template<class T, class commpFun>
int merge_sort(T* ptr, int size, commpFun commp)
{
	mergeSort(ptr, 0, size -1, commp);
	return 0;
}

/*
* 快速排序
*/
template<class T, class commpFun>
int adjustArray(T* ptr, int first, int last, commpFun commp)
{
	int x = ptr[first];
	int i = first; int j = last;
	while(i < j) {
		while(i < j && commp(x, ptr[j])) { //位置i被挖坑
			j --;
		}
		if(i < j) {
			ptr[i++] = ptr[j];
		}

		while(i < j && commp(ptr[i], x)) { //位置j被挖坑
			i ++;
		}
		if(i < j) {
			ptr[j--] = ptr[i];
		}
	}
	ptr[i] = x;
	return i;
}

template<class T, class commpFun>
int quickSort(T* ptr, int first, int last, commpFun commp)
{
	if(first < last) { //队列不止一个元素，分治
		int mid = adjustArray(ptr, first, last, commp);
		quickSort(ptr, first, mid - 1, commp);
		quickSort(ptr, mid + 1, last, commp);
	}	

	return 0;
}

template<class T, class commpFun>
int quick_sort(T* ptr, int size, commpFun commp)
{
	quickSort(ptr, 0, size - 1, commp);
	return 0;
}

/*
* 堆排序
* 大堆、小堆取决于commpFun
*/
template<class T, class commpFun>
int adjustHeapFixDown(T* ptr, int size, int index, commpFun commp)
{
	int i = 2 * index + 1; //左子节点
	while(i < size) {
		if(i + 1 < size && commp(ptr[i+1], ptr[i])) i ++; //找出最小的子节点
		if(!commp(ptr[i], ptr[index])) break; //左右子节点都比自己小，调整结束，退出循环
		sfc_swap(ptr[index], ptr[i]); //子节点比自己小，和最小的子节点交换
		index = i;
		i = 2 * i + 1;
	}
	return 0;
}

template<class T, class commpFun>
int heap_sort(T* ptr, int size, commpFun commp)
{
	//将数组堆化
	for(int i = size / 2 - 1; i >= 0; i --) {
		adjustHeapFixDown(ptr, size, i, commp);
	}

	cout<<"heaped array: "<<endl;
	echo_array(ptr, size);

	//对堆化的数组进行排序
	for(int i = 0; i < size; i++) {
		sfc_swap(ptr[0], ptr[size-1-i]);
		adjustHeapFixDown(ptr, size-1-i, 0, commp);
	}

	return 0;
}





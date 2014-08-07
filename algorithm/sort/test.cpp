#include <iostream>

#include "common.h"
#include "sort.h"

using namespace std;

int main(int argc, char* argv[])
{
	int* vals = new int[10];
	shared_ptr<int> sp(vals);

	vals[0] = 89;
	vals[1] = 76;
	vals[2] = 12;
	vals[3] = 0;
	vals[4] = 91;
	vals[5] = 88;
	vals[6] = 34;
	vals[7] = 21;
	vals[8] = 56;
	vals[9] = 92;

	/*vals[0] = 9;
	vals[1] = 12;
	vals[2] = 17;
	vals[3] = 30;
	vals[4] = 50;
	vals[5] = 20;
	vals[6] = 60;
	vals[7] = 65;
	vals[8] = 4;
	vals[9] = 19;*/

	cout<<"before sort: "<<endl;
	echo_array(vals, 10);

	// insert_sort2(vals, 10, ascend_comp);
	// shell_sort(vals, 10, ascend_comp);
	// select_sort(vals, 10, ascend_comp);
	// merge_sort(vals, 10, ascend_comp);
	// quick_sort(vals, 10, ascend_comp);
	heap_sort(vals, 10, descend_comp);

	cout<<"after sort: "<<endl;
	echo_array(vals, 10);


	return 0;
}

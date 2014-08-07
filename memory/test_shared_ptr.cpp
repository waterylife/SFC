#include <memory>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "sfc_shared_ptr.hpp"

using namespace std;
using namespace boost;
using namespace sfc;

void test_sfc_shared_ptr()
{
	sfc_shared_ptr<int> sp1(new int(86));
	cout<<"sp1 content: "<<*sp1<<"sp1 use count: "<<sp1.use_count()<<endl;

	sfc_shared_ptr<int> sp2;
	sp2 = sp1;
	cout<<"sp1 content: "<<*sp1<<"sp1 use count: "<<sp1.use_count()<<endl;
	cout<<"sp2 content: "<<*sp2<<"sp2 use count: "<<sp2.use_count()<<endl;
}

int main()
{
	test_sfc_shared_ptr();

	return 0;
}
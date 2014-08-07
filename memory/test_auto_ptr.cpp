#include <memory>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "sfc_auto_ptr.hpp"

using namespace std;

void test_sfc_auto_ptr()
{
  int* p = new int(676);

  sfc_auto_ptr<int> a1(p);
  sfc_auto_ptr<int> a2(p);

  printf("a1 content: %d\n", *a1);
  printf("a2 content: %d\n", *a2);

  a1 = a2;

  printf("a1 content: %d\n", *a1);

}

void test_auto_ptr()
{
  auto_ptr<int> a1(new int(999));
  auto_ptr<long int> a2(new (long int)(888));

  printf("a1 content: %d\n", *a1);
  printf("a2 content: %d\n", *a2);

  // a1 = a2;

  printf("a1 content: %d\n", *a1);
}

int main()
{
    test_sfc_auto_ptr();
    // test_auto_ptr();

 /*   int* p = new int(897);
    auto_ptr<int> a1;
    auto_ptr<int> a2(p);

    int* ptr = (int*)malloc(sizeof(int));*/
    /*delete ptr;
    delete ptr;*/
    // free(ptr);
    // free(ptr);

  /*  printf("a1 content: %d, a1 pointer addr: %x, a1 addr: %x\n", *a1, a1.get(), &a1);
    printf("a2 content: %d, a2 pointer addr: %x, a2 addr: %x\n", *a2, a2.get(), &a2);*/

    // a1 = a2;

/*    printf("a1 content: %d, a1 pointer addr: %x, a1 addr: %x\n", *a1, a1.get(), &a1);
    printf("a2 content: %d, a2 pointer addr: %x, a2 addr: %x\n", *a2, a2.get(), &a2);*/
    // *a1 = 980;


    return 0;
}
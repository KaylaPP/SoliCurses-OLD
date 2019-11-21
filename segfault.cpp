#include <iostream>
using namespace std;

int main()
{
	int one = 1;
	int *pointer = &one;
	delete pointer;
	cout << &pointer;
	return 0;
}

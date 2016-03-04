#include <systemcpp.h>

int addi(int a, int b) { return 42; }
long addl(long a, long b) { return 52; }
long long addll(long long a, long long b) { return 62; }

// call from C++ with parameter overloading
int add(int a, int b) { return addi(a,b); }
long add(long a, long b) { return addl(a,b); }
long long add(long long a, long long b) { return addll(a,b); }

void test() {
	int c = 2;
	int* d = &c;
	add(c,*d);
}
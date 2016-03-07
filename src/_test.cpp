// TODO: probably should leave c++ to compiling for the full app target??
// - though if we could use lambdas and a few other things that may be interesting?
// - but it may just make the hardware interface more opaque
// TODO: Possibly have a common system.h and then systemc.h and
#include <system.h>
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

// Test compiling classes
struct entity_component {};
struct entity {
    entity_component components[];
};
struct entity_system {
    entity_component components_this_handles[];
    void update(real64 dt);
};

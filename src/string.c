#include <system.h>

typedef struct {
    const char* bytes;
    u32 count;
} st_str;


typedef struct {
    void* elements;
    u32 size;
} dyn_array;

//
////enum class StringError;
////StringError k_strncpy();
//
//// NOTE: C99 allows runtime array decl
//// https://gcc.gnu.org/onlinedocs/gcc/Variable-Length.html
//void inc_each(int n, u8[n] pArr)
//{
//    for(int i=0; i<n; ++i) {
//        pArr[i]++;
//    }
//}
//
//void test_c99_arrays(int i, int j)
//{
//    u32 arr[i];
//    u8 another[j];
//
//    // TODO: :( sad face :(
//    // notice we'd rather pass a struct arround since we have to still track the count with c99
//    inc_each(j, arr);
//    inc_each(i, arr);
//}
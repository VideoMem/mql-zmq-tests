#include "hello_world.hpp"

void string_marshall(const string_t* s, const string &d) {
    size_t size = 120;
    printf("%d\n", size);
    const string_t** cpy = new string_t[size][1];
    char*            dst = new char[size];
    mbstate_t* state = 0;
    size_t cpysize = wcsrtombs(dst,cpy,size,state);
    size_t im = 0;
    d.copy(dst, im, 0);
    delete [] cpy;
    delete [] dst;
    printf(dst);
}

void string_unmarshall(const string &s, const string_t* d) {


}

int_t LPCCALL int_echo(int_t a) {
    return a;
}

size_t LPCCALL string_echo(string_t* a, string_t* r) {
    string test;
    string_marshall(a,test);
    return wcslen(wcscpy(r,a));
}
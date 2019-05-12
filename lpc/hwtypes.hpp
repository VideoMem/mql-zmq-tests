#include <cinttypes>
#include <iostream>
using namespace std;

#define widptr_t uint32_t
#define int_t uint32_t
#define string_t wchar_t


void string_marshall(const string_t* s, string &d) {
    wstring wcstring = s;
    char cstr[sizeof(wcstring)];
    wcstombs(cstr, s, sizeof(wcstring));
    d = cstr;
}

void string_unmarshall(const string &s, string_t* d) {
    string_t wcstr[sizeof(s)];
    mbstowcs(wcstr, s.c_str(), sizeof(wcstr));
    wstring wcstring = wcstr;
    wcstring.copy(d, sizeof(wcstr), 0);
}
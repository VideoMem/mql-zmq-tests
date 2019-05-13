#include <cinttypes>
#include <iostream>
#include <sstream>
using namespace std;

//Cross DLL boundaries types.
#define int_t    uint32_t
#define widptr_t int_t
#define string_t wchar_t

size_t string_size(const string& s) {
    int mul = sizeof(string_t) / sizeof(char);
    return s.size() * mul;
}

size_t wstring_size(const wstring& s) {
    int mul = sizeof(string_t) / sizeof(char);
    return s.size() * mul;
}

//String marshalling from DLL call string_t* argument to std::string
void string_marshall(const string_t* s, string &d) {
    wstring wcstring = s;
    char cstr[wstring_size(wcstring)];
    wcstombs(cstr, s, wstring_size(wcstring));
    d = cstr;
}

//String de-marshalling from std::string to string_t*  DLL call argument
void string_unmarshall(const string &s, string_t* d) {
    string_t wcstr[string_size(s)];
    mbstowcs(wcstr, s.c_str(), string_size(s));
    wstring wcstring = wcstr;
    wcscpy(d,wcstring.c_str());
}
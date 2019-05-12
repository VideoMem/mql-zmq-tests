#include "lpclient.hpp"

int_t LPCCALL int_echo(int_t a) {
    return a;
}

size_t LPCCALL string_echo(string_t* src, string_t* dst) {
    string stdstring;
    string_marshall(src, stdstring);
    string_t wstring[sizeof(stdstring)] = {0};
    string_unmarshall(stdstring, wstring);
    return wcslen(wcscpy(dst,wstring));
}
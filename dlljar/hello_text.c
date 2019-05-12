/* hello_test.c

   Demonstrates using the function imported from the DLL, in a flexible and
   elegant way.
*/

#include "hello_world.hpp"
#include <iostream>

int main(int argc, char** argv) {
  printf("%d\n", int_echo(42));
  string_t reply[] = L"            ";
  string_t source[] = L"Hello\n";
  size_t size = string_echo(source, reply);
  wprintf(source);
  wprintf(reply);
  printf("reply len: %d\n", size);
  return EXIT_SUCCESS;
}

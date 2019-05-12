/* hello_world.h

   Declares the functions to be imported by our application, and exported by our
   DLL, in a flexible and elegant way.
*/
#include "hwtypes.hpp"

#ifdef _WIN32

  /* You should define ADD_EXPORTS *only* when building the DLL. */
  #ifdef ADD_EXPORTS
    #define LPCAPI __declspec(dllexport)
  #else
    #define LPCAPI __declspec(dllimport)
  #endif

  /* Define calling convention in one place, for convenience. */
  #define LPCCALL __cdecl

#else /* _WIN32 not defined. */

  /* Define with no value on non-Windows OSes. */
  #define LPCAPI
  #define LPCCALL

#endif

/* Make sure functions are exported with C linkage under C++ compilers. */

#ifdef __cplusplus
extern "C"
{
#endif

/* Declare our <type> echo function using the above definitions. */
LPCAPI int_t LPCCALL int_echo(int_t a);
LPCAPI size_t LPCCALL string_echo(string_t* a, string_t* r);

#ifdef __cplusplus
} // __cplusplus defined.
#endif
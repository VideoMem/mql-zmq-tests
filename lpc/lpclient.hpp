#include "lpclient/hwtypes.hpp"

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
LPCAPI int_t  LPCCALL int_echo                  (int_t a);
LPCAPI size_t LPCCALL string_echo               (string_t* a, string_t* r);
LPCAPI size_t LPCCALL char_echo                 (char* a, char* r, int_t len);
LPCAPI void   LPCCALL worker_add                (char* name, int_t nlen, char* address, int_t alen, widptr_t& nid);
LPCAPI void   LPCCALL worker_echo               (widptr_t id, char* msg, int_t size);
LPCAPI size_t LPCCALL worker_send               (widptr_t id, char* send, int_t size);
LPCAPI int_t  LPCCALL worker_getreplysize       (widptr_t id);
LPCAPI void   LPCCALL worker_getreply           (widptr_t id, char* reply, int_t size);
LPCAPI void   LPCCALL worker_setaddr            (widptr_t id, char* address, int_t size);
LPCAPI void   LPCCALL worker_setname            (widptr_t id, char* name, int_t size);
LPCAPI void   LPCCALL worker_getname            (widptr_t id, char* name, int_t &size);
LPCAPI void   LPCCALL worker_getLastError       (widptr_t id, int_t &err);
LPCAPI void   LPCCALL worker_getLErrContext     (widptr_t id, char* err, int_t len);
LPCAPI void   LPCCALL worker_setRequestTimeout  (widptr_t id, int_t timeout);
LPCAPI void   LPCCALL worker_setRequestRetries  (widptr_t id, int_t value);
LPCAPI void   LPCCALL worker_deinit             (void);

#ifdef __cplusplus
} // __cplusplus defined.
#endif



#ifndef __SIGNED_SIZE_H__
#define __SIGNED_SIZE_H__

/* Cross-platform definition of ssize_t (signed size_t).
 * This is a signed integer that can hold all possible values of a size_t,
 * so it can be used as the return type for functions that need to return
 * "either a size_t or -1" to indicate errors.
 */

#ifndef signed_size
#ifdef _WIN32
#include <BaseTsd.h>
#define signed_size SSIZE_T
#else
#include <stddef.h>
#define signed_size ptrdiff_t
#endif
#endif

#endif

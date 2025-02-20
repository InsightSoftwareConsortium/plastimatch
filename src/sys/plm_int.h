/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_int_h_
#define _plm_int_h_

#include "plm_config.h"

#if defined (_MSC_VER) && (_MSC_VER < 1600)
#include "msinttypes/stdint.h"
#else
#include <stdint.h>
#endif

/* These are non-standard */
#ifndef UINT32_T_MAX
#define UINT32_T_MAX (0xffffffff)
#endif
#ifndef INT32_T_MAX
#define INT32_T_MAX (0x7fffffff)
#endif
#ifndef INT32_T_MIN
#define INT32_T_MIN (-0x7fffffff - 1)
#endif

/* The data type plm_long is a signed integer with the same size as size_t.
   It is equivalent to the POSIX idea of ssize_t.  It is used for 
   OpenMP 2.0 loop variables which must be signed. */
#if (CMAKE_SIZEOF_SIZE_T == 8)
typedef int64_t plm_long;
#elif (CMAKE_SIZEOF_SIZE_T == 4)
typedef int32_t plm_long;
#else
#error "Unexpected value for sizeof(size_t)"
#endif

#endif

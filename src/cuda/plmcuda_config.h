/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmcuda_config_h__
#define __plmcuda_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmcuda_EXPORTS
#   define PLMCUDA_C_API EXTERNC __declspec(dllexport)
#   define PLMCUDA_API __declspec(dllexport)
# else
#   define PLMCUDA_C_API EXTERNC __declspec(dllimport)
#   define PLMCUDA_API __declspec(dllimport)
# endif
#else
# define PLMCUDA_C_API EXTERNC 
# define PLMCUDA_API 
#endif

#endif

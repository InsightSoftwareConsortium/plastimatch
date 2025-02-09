##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_sys)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##  GCS 2012-09-28: Please include non-private header files as well
##-----------------------------------------------------------------------------
set (PLMSYS_LIBRARY_SRC
  compiler_warnings.h
  delayload.h
  dir_list.cxx dir_list.h
  double_align8.h
  dlib_threads.cxx dlib_threads.h 
  file_util.cxx file_util.h
  logfile.cxx logfile.h
  option_range.cxx option_range.h 
  path_util.cxx path_util.h
  plm_endian.cxx plm_endian.h
  plm_exception.h 
  plm_fwrite.cxx plm_fwrite.h
  plm_int.h
  plm_macros.h
  plm_math.h
  plm_return_code.h
  plm_sleep.cxx plm_sleep.h 
  plm_timer.cxx plm_timer.h plm_timer_p.h
  plm_va_copy.h
  print_and_exit.cxx print_and_exit.h
  smart_pointer.h
  string_util.cxx string_util.h
  )

if (PLM_CONFIG_LEGACY_CUDA_DELAYLOAD)
list (APPEND PLMSYS_LIBRARY_SRC
  delayload_legacy.c delayload_legacy.h
  )
else ()
list (APPEND PLMSYS_LIBRARY_SRC
  cuda_delayload.cxx cuda_delayload.h
  )
endif ()

set (PLMSYS_LIBRARY_HEADERS "${PLM_BINARY_DIR}/plm_config.h")
foreach (ARG ${PLMSYS_LIBRARY_SRC})
  string (REGEX MATCH ".*\\.h$" TMP "${ARG}")
  if (TMP)
    list (APPEND PLMSYS_LIBRARY_HEADERS "${TMP}")
  endif ()
endforeach ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMSYS_LIBRARY_DEPENDENCIES
  ${DLIB_LIBRARIES}
  )
if (LIBDL_FOUND)
    set (PLMSYS_LIBRARY_DEPENDENCIES
	${PLMSYS_LIBRARY_DEPENDENCIES}
	dl
	)
endif ()
if (CYGWIN OR NOT WIN32)
    find_library (pthreadlib pthread)
    if (pthreadlib)
	set (PLMSYS_LIBRARY_DEPENDENCIES
	    ${PLMSYS_LIBRARY_DEPENDENCIES}
	    ${pthreadlib}
	    )
    endif ()
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmsys
  "${PLMSYS_LIBRARY_SRC}" 
  "${PLMSYS_LIBRARY_DEPENDENCIES}"
  ""
  "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
  "${PLMSYS_LIBRARY_HEADERS}")

# We need to add PLM_BINARY_DIR to the DT_RUNPATH on linux for delayload
# to find the dynamically loaded libraries
if (PLM_USE_GPU_PLUGINS)
  if (CMAKE_VERSION VERSION_LESS "3.8")
    link_directories ("${PLM_BINARY_DIR}")
  else ()
    set_target_properties (plmsys PROPERTIES BUILD_RPATH "${PLM_BINARY_DIR}")
  endif ()
endif ()

# - Find Slicer
# Find the Slicer includes and library
#
# The user should set Slicer_DIR to the directory containing either 
# Slicer3Config.cmake or SlicerConfig.cmake
#
#  SLICER_FOUND       - true if found
#  SLICER_IS_SLICER3  - true if we have Slicer vesion 3.X
#  SLICER_IS_SLICER4  - true if we have Slicer vesion 4.X

if (SLICER_INCLUDE_DIR)
  # Already in cache, be silent
  set (Slicer_FIND_QUIETLY TRUE)
endif ()

# The Slicer4 config file complains if these are set.
unset (ITK_DIR CACHE)
unset (QT_QMAKE_EXECUTABLE CACHE)
unset (VTK_DIR CACHE)

# Find SlicerConfig.cmake or Slicer3Config.cmake
unset (SLICER_CONFIG_FILE CACHE)
if (Slicer_DIR OR Slicer3_DIR)
  find_file (SLICER_CONFIG_FILE SlicerConfig.cmake
    "${Slicer_DIR}" "${Slicer3_DIR}")
  if (SLICER_CONFIG_FILE)
    set (SLICER_IS_SLICER3 FALSE)
  else ()
    find_file (SLICER_CONFIG_FILE Slicer3Config.cmake
      "${Slicer_DIR}" "${Slicer3_DIR}")
    if (SLICER_CONFIG_FILE)
      set (SLICER_IS_SLICER3 TRUE)
    endif ()
  endif ()
endif ()

# This sets Slicer_DIR in the cache.  I couldn't find any other way to do it.
if (SLICER_CONFIG_FILE)
  get_filename_component (SLICER_CONFIG_FILE_DIR
    "${SLICER_CONFIG_FILE}" PATH)
  set (Slicer_DIR "${SLICER_CONFIG_FILE_DIR}"
    CACHE PATH "Directory with SlicerConfig.cmake or Slicer3Config.cmake"
    FORCE)
else ()
  set (Slicer_DIR "Slicer_DIR-NOTFOUND"
    CACHE PATH "Directory with SlicerConfig.cmake or Slicer3Config.cmake"
    FORCE)
endif ()

message (STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
if (SLICER_CONFIG_FILE)
  include ("${SLICER_CONFIG_FILE}")
endif ()

# handle the QUIETLY and REQUIRED arguments and set SLICER_FOUND to TRUE if 
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (SLICER DEFAULT_MSG Slicer_DIR)

if (SLICER_FOUND)

  if (SLICER_IS_SLICER3)
    set (SLICER_IS_SLICER4 FALSE)
  else ()  
    set (SLICER_IS_SLICER4 TRUE)
  endif ()

  if (SLICER_IS_SLICER3)
    ## Slicer 3: convert old names to new names
    set (Slicer_Base_INCLUDE_DIRS "${Slicer3_Base_INCLUDE_DIRS}")
    set (Slicer_Libs_INCLUDE_DIRS "${Slicer3_Libs_INCLUDE_DIRS}")
    set (Slicer_Base_LIBRARIES "${Slicer3_Base_LIBRARIES}")
    set (Slicer_Libs_LIBRARIES "${Slicer3_Libs_LIBRARIES}")
    set (Slicer_USE_FILE "${Slicer3_USE_FILE}")
  else ()
    ## Slicer 4: include missing cmake scripts (Slicer3 stuff)
    include (Slicer3Macros)
    include (Slicer3PluginsMacros)
    ## Slicer 4: include missing cmake scripts (loadable module stuff)
    include (vtkMacroKitPythonWrap)
    include (ctkMacroWrapPythonQt)
    include (ctkMacroCompilePythonScript)
  endif ()
    ## Always set shared libs on
  set (BUILD_SHARED_LIBS ON)
endif ()
message (STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

mark_as_advanced (SLICER_CONFIG_FILE)

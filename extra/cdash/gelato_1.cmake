SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/work/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/gelato_1/plastimatch")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
BUILDNAME:STRING=01-lin64-Pisr-Cd-gcc4.4.5

//Name of the computer/site where compile is being run
SITE:STRING=gelato

//The directory containing ITKConfig.cmake.  This is either the
// root of the build tree, or PREFIX/lib/InsightToolkit for an
// installation.
ITK_DIR:PATH=/home/gcs6/build/itk-3.20.1

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=ON
")

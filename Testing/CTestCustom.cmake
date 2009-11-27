
SET (CUDA_FOUND @CUDA_FOUND@)
SET (BROOK_FOUND @BROOK_FOUND@)

#SET (REDUCED_TEST ON)
SET (REDUCED_TEST OFF)

IF (NOT CUDA_FOUND)
  SET (CTEST_CUSTOM_TESTS_IGNORE
    ${CTEST_CUSTOM_TESTS_IGNORE}
    "fdk-cuda"
    "fdk-cuda-stats"
    "fdk-cuda-check"
    "plm-bspline-cuda" 
    "plm-bspline-cuda-stats" 
    "plm-bspline-cuda-check" 
    )
ENDIF (NOT CUDA_FOUND)

IF (NOT BROOK_FOUND)
  SET (CTEST_CUSTOM_TESTS_IGNORE
    ${CTEST_CUSTOM_TESTS_IGNORE}
    "fdk-brook"
    "fdk-brook-stats"
    "fdk-brook-check"
    )
ENDIF (NOT BROOK_FOUND)

## Don't delete from the list, comment out instead.
IF (REDUCED_TEST)
  SET (CTEST_CUSTOM_TESTS_IGNORE
    ${CTEST_CUSTOM_TESTS_IGNORE}
    "synth-test-1"
    "synth-test-2"
    "synth-test-3"
    "bspline-bxf"
    "bspline-bxf-check"
    "bspline-a"
    "bspline-a-check"
    "bspline-b"
    "bspline-b-check"
    "bspline-c"
    "bspline-c-check"
    "bspline-d"
    "bspline-d-check"
    "bspline-e"
    "bspline-e-check"
    "bspline-f"
    "bspline-f-check"
    "drr"
    "fdk-cpu"
    "fdk-cpu-stats"
    "fdk-cpu-check"
    "plm-resample.a"
    "plm-warp-a"
    "plm-warp-b"
    "plm-warp-c"
    "tps-warp"
    "xf-to-xf"
    )
ENDIF (REDUCED_TEST)

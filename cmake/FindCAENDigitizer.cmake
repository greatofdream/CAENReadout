find_path(CAENDigitizer_INCLUDE_DIR libCAENDigitizer.h /usr/include/ /usr/local/include ${CMAKE_SOURCE_DIR}/ModuleMode)
find_library(CAENDigitizer_LIBRARY NAMES CAENDigitizer PATHS /usr/lib/CAENDigitizer /usr/local/lib/CAENDigitizer ${CMAKE_SOURCE_DIR}/ModuleMode)

if (CAENDigitizer_INCLUDE_DIR AND CAENDigitizer_LIBRARY)
    set(CAENDigitizer_FOUND TRUE)
endif (CAENDigitizer_INCLUDE_DIR AND CAENDigitizer_LIBRARY)

# Self-contained onnxruntime: download the prebuilt CPU release and expose it as an
# imported onnxruntime::onnxruntime target. Adapted from ossia/score-addon-onnx
# (CPU-only, no CUDA / score install plumbing).
include(FetchContent)

set(ONNXRUNTIME_VERSION "1.24.1")
set(_ort_base "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}")
if(WIN32)
  set(ONNXRUNTIME_URL "${_ort_base}/onnxruntime-win-x64-${ONNXRUNTIME_VERSION}.zip")
elseif(APPLE)
  set(ONNXRUNTIME_URL "${_ort_base}/onnxruntime-osx-arm64-${ONNXRUNTIME_VERSION}.tgz")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64.*")
  set(ONNXRUNTIME_URL "${_ort_base}/onnxruntime-linux-aarch64-${ONNXRUNTIME_VERSION}.tgz")
else()
  set(ONNXRUNTIME_URL "${_ort_base}/onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}.tgz")
endif()

FetchContent_Declare(onnxruntime URL "${ONNXRUNTIME_URL}")
FetchContent_MakeAvailable(onnxruntime)

find_library(onnxruntime_LIBRARY NAMES onnxruntime
  PATHS "${onnxruntime_SOURCE_DIR}/lib" NO_DEFAULT_PATH)
find_path(onnxruntime_INCLUDE_DIRS NAMES onnxruntime_cxx_api.h
  PATHS "${onnxruntime_SOURCE_DIR}/include" NO_DEFAULT_PATH)
if(WIN32)
  find_file(onnxruntime_DLL onnxruntime.dll
    PATHS "${onnxruntime_SOURCE_DIR}/lib" NO_DEFAULT_PATH)
endif()

if(NOT onnxruntime_LIBRARY OR NOT onnxruntime_INCLUDE_DIRS)
  message(FATAL_ERROR "onnxruntime not found in ${onnxruntime_SOURCE_DIR}")
endif()

add_library(onnxruntime SHARED IMPORTED)
if(WIN32)
  set_target_properties(onnxruntime PROPERTIES
    IMPORTED_LOCATION "${onnxruntime_DLL}"
    IMPORTED_IMPLIB "${onnxruntime_LIBRARY}")
else()
  set_target_properties(onnxruntime PROPERTIES IMPORTED_LOCATION "${onnxruntime_LIBRARY}")
endif()
target_include_directories(onnxruntime INTERFACE "${onnxruntime_INCLUDE_DIRS}")
# Avendish builds with -fno-exceptions/-fno-rtti; make the onnxruntime C++ API match.
target_compile_definitions(onnxruntime INTERFACE ORT_NO_EXCEPTIONS=1)
add_library(onnxruntime::onnxruntime ALIAS onnxruntime)

# The dump/standalone executables link onnxruntime and run during the build; make the
# shared library findable (CI forces install-rpath). Set before the targets are created.
list(APPEND CMAKE_BUILD_RPATH "${onnxruntime_SOURCE_DIR}/lib")
list(APPEND CMAKE_INSTALL_RPATH "${onnxruntime_SOURCE_DIR}/lib")

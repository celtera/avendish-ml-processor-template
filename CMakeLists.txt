cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(MyOnnx CXX)

# Use the Avendish the host already provides (e.g. ossia score); otherwise fetch it.
# The same CMakeLists builds this as an ossia/score add-on or as a standalone object.
find_package(Avendish QUIET)
if(NOT Avendish_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    Avendish
    GIT_REPOSITORY "https://github.com/celtera/avendish"
    GIT_TAG 0f706ab)
  FetchContent_Populate(Avendish)
  list(APPEND CMAKE_PREFIX_PATH "${avendish_SOURCE_DIR}")
  find_package(Avendish REQUIRED)
endif()

# Downloads the prebuilt onnxruntime and exposes onnxruntime::onnxruntime.
include(cmake/onnxruntime.cmake)

avnd_addon_init(NAME MyOnnx)

# CATEGORY object: ossia, Max/MSP, Pure Data, Python, TouchDesigner (CHOP message),
# Godot. DEPENDENCIES onnxruntime::onnxruntime is linked so it propagates to the score
# base target and to every standalone back-end (each compiles the onnxruntime C++ API
# directly). In a score build only the ossia process is produced.
avnd_addon_object(
  BASE MyOnnx
  C_NAME my_onnx_processor
  CLASS MyOnnxProcessor
  # Explicit back-end list (the object-category set) including dump — the
  # plain-C++ introspection target the portability CI builds.
  BACKENDS dump ossia python pd max standalone wasm example_host
           touchdesigner:CHOP_MESSAGE godot:NODE
  DEPENDENCIES onnxruntime::onnxruntime
  MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/Processor.hpp"
  SOURCES
    src/Model.cpp
    src/Model.hpp
    src/Processor.hpp)

avnd_addon_finalize(NAME MyOnnx UUID 2cd49539-0cfe-4e43-bade-16d718ebc747 VERSION 1.0.0)

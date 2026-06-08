cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(MyOnnx CXX)

include(dependencies.cmake)
include(cmake/onnxruntime.cmake)

add_library(MyOnnxProcessor STATIC
  src/Model.cpp
  src/Model.hpp
  src/Processor.hpp
)
target_link_libraries(MyOnnxProcessor PUBLIC onnxruntime::onnxruntime)

# avnd_make_object instantiates the data / message back-ends (no audio plug-in formats):
# ossia, Max/MSP, Pure Data, Python, TouchDesigner (CHOP message), Godot.
avnd_make_object(
  TARGET MyOnnxProcessor
  MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/Processor.hpp"
  MAIN_CLASS MyOnnxProcessor
  C_NAME my_onnx_processor
)

# Each back-end compiles the header (and thus the onnxruntime C++ API) directly, so it
# must see onnxruntime's headers and link its library.
foreach(suffix max pd CHOP_MESSAGE_td NODE_godot python standalone example_host dump ossia)
  if(TARGET MyOnnxProcessor_${suffix})
    target_link_libraries(MyOnnxProcessor_${suffix} PRIVATE onnxruntime::onnxruntime)
  endif()
endforeach()

# Workaround: avnd_make_godot doesn't always resolve the godot-cpp target, so re-link it
# here to propagate godot-cpp's generated include dirs (cf. the other templates).
if(TARGET MyOnnxProcessor_NODE_godot AND TARGET godot-cpp)
  target_link_libraries(MyOnnxProcessor_NODE_godot PRIVATE godot-cpp)
endif()

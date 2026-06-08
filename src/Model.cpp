// The MyOnnxProcessor implementation lives entirely in Model.hpp (inline), because the
// Avendish back-ends compile the header directly and several do not link this library.
// This translation unit only exists so the CMake target has a source to compile.
#include "Model.hpp"

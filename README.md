# Avendish ML / ONNX inference template

A canonical template for making **ONNX inference** objects with
[Avendish](https://github.com/celtera/avendish): a single C++ object that runs a `.onnx`
model with [onnxruntime](https://onnxruntime.ai/) and is compiled, without rewrite, to
the host formats that have a real object model.

It is the ML counterpart of the
[audio](https://github.com/celtera/avendish-audio-processor-template),
[video](https://github.com/celtera/avendish-video-processor-template),
[geometry](https://github.com/celtera/avendish-geometry-processor-template) and
[data](https://github.com/celtera/avendish-data-processor-template) templates.

## What gets built

`avnd_addon_object(... CATEGORY object)` instantiates the object back-ends — **not** the audio plug-in
formats (VST3 / CLAP):

| Back-end | SDK required |
|---|---|
| Max/MSP, Pure Data | Max SDK, PureData headers |
| TouchDesigner (CHOP message) | TouchDesigner Custom Operator SDK |
| Godot (`GDExtension` node) | none — fetched by CMake |
| Python (extension module) | pybind11 |
| ossia score | libossia |

**onnxruntime** is downloaded by CMake (`cmake/onnxruntime.cmake`, the prebuilt CPU
release) and linked into every back-end. Avendish has no ONNX helper, so the object uses
the onnxruntime C++ API directly; for richer (vision) models see
[`ossia/score-addon-onnx`](https://github.com/ossia/score-addon-onnx).

## The object

`src/Model.hpp` implements `MyOnnxProcessor`: set **Model** to a `.onnx` path, feed a
float vector on **Input**, and it runs the model's single input/output and emits the
result on **Output**. The session is (re)loaded when the path changes.

A tiny model is bundled: `model/scale_by_two.onnx` computes `y = 2*x`
(so `[1,2,3]` → `[2,4,6]`). Regenerate / replace it with your own; change the ports in
`Model.hpp` to match your model's shapes.

## Building

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

CMake downloads onnxruntime and `godot-cpp` automatically. Pass `-DAVND_MAXSDK_PATH=…`,
`-DTOUCHDESIGNER_SDK_PATH=…`, `-DCMAKE_INCLUDE_PATH=…/pure-data/src`,
`-Dpybind11_DIR=…` to enable the other back-ends — see the
[Github actions workflow](.github/workflows/build_cmake.yml) (shared
[`ossia/actions/avendish-template`](https://github.com/ossia/actions) action).

## Notes

- onnxruntime's C++ API is compiled with `ORT_NO_EXCEPTIONS` to match Avendish's
  `-fno-exceptions` build; model/inference errors therefore abort rather than throw, so
  feed valid models and shapes.
- At runtime each plug-in needs the onnxruntime shared library next to it (it is fetched
  into the build tree under `_deps/onnxruntime-src/lib`).

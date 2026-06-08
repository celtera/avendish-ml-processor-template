#pragma once
#include <halp/controls.hpp>
#include <halp/meta.hpp>

#include <onnxruntime_cxx_api.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// MyOnnxProcessor runs inference on a .onnx model with onnxruntime: it takes a float
// vector in, runs the model's single input/output, and produces a float vector out.
// It is a "data" object (avnd_make_object) — Max/MSP, Pure Data, TouchDesigner,
// Godot, Python, ossia — not an audio plug-in.
//
// Ship a model and point "Model" at it; the bundled model/scale_by_two.onnx computes
// y = 2*x (so [1,2,3] -> [2,4,6]). The model is (re)loaded whenever the path changes.
//
// onnxruntime is fetched by CMake (cmake/onnxruntime.cmake). There is no Avendish ONNX
// helper — this template uses the onnxruntime C++ API directly; see ossia/score-addon-onnx
// for richer (vision) models.
class MyOnnxProcessor
{
public:
  halp_meta(name, "My ONNX Processor")
  halp_meta(c_name, "my_onnx_processor")
  halp_meta(category, "AI")
  halp_meta(author, "Avendish")
  halp_meta(description, "Run inference on a .onnx model (float vector in -> out)")

  // CHANGE THIS !! (uuidgen)
  halp_meta(uuid, "9a2c1e74-3b5f-4d80-8e16-7c4f0d9a2b53")

  struct ins
  {
    // Path to the .onnx model file. Reloaded when it changes.
    halp::val_port<"Model", std::string> model_path;

    // Input tensor (flattened); runs inference on arrival.
    struct : halp::val_port<"Input", std::vector<float>>
    {
      void update(MyOnnxProcessor& self) { self(); }
    } input;
  } inputs;

  struct
  {
    // Inference result (flattened).
    halp::val_port<"Output", std::vector<float>> output;
  } outputs;

  void operator()();

private:
  std::string loaded_path, in_name, out_name;
  std::unique_ptr<Ort::Env> env;
  std::unique_ptr<Ort::Session> session;
};

inline void MyOnnxProcessor::operator()()
{
  // (Re)load the session when the model path changes.
  const std::string& path = inputs.model_path.value;
  if(path != loaded_path)
  {
    session.reset();
    env.reset();
    loaded_path.clear();
    if(!path.empty())
    {
      env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "avnd-onnx");
      Ort::SessionOptions opts;
      opts.SetIntraOpNumThreads(1);
#if defined(_WIN32)
      std::wstring wpath(path.begin(), path.end());
      session = std::make_unique<Ort::Session>(*env, wpath.c_str(), opts);
#else
      session = std::make_unique<Ort::Session>(*env, path.c_str(), opts);
#endif
      Ort::AllocatorWithDefaultOptions alloc;
      in_name = session->GetInputNameAllocated(0, alloc).get();
      out_name = session->GetOutputNameAllocated(0, alloc).get();
      loaded_path = path;
    }
  }

  auto& in = inputs.input.value;
  if(!session || in.empty())
    return;

  const int64_t shape[1] = {static_cast<int64_t>(in.size())};
  auto mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::Value in_tensor
      = Ort::Value::CreateTensor<float>(mem, in.data(), in.size(), shape, 1);

  const char* in_names[1] = {in_name.c_str()};
  const char* out_names[1] = {out_name.c_str()};
  auto outs = session->Run(
      Ort::RunOptions{nullptr}, in_names, &in_tensor, 1, out_names, 1);

  const float* od = outs[0].GetTensorData<float>();
  const std::size_t n = outs[0].GetTensorTypeAndShapeInfo().GetElementCount();
  outputs.output.value.assign(od, od + n);
}

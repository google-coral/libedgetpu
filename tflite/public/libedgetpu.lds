VER_1.0 {
  global:
    extern "C++" {
      edgetpu::*;
    };
    /* Functions to support edge TPU custom op from TFLite Python API, check
     * `//third_party/tensorflow/lite/python/interpreter.py` for the spec.
     */
    tflite_plugin_*;
    /* Edge TPU C API functions.
     */
    edgetpu_*;
  local:
    *;
};

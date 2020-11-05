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

    /* Export `new` and `delete` variants to workaround b/167358360.
    */
    _ZdaPv;
    _ZdaPvRKSt9nothrow_t;
    _ZdaPvSt11align_val_t;
    _ZdaPvSt11align_val_tRKSt9nothrow_t;
    _ZdaPvm;
    _ZdaPvmSt11align_val_t;
    _ZdlPv;
    _ZdlPvRKSt9nothrow_t;
    _ZdlPvSt11align_val_t;
    _ZdlPvSt11align_val_tRKSt9nothrow_t;
    _ZdlPvm;
    _ZdlPvmSt11align_val_t;
    _Znam;
    _ZnamRKSt9nothrow_t;
    _ZnamSt11align_val_t;
    _ZnamSt11align_val_tRKSt9nothrow_t;
    _Znwm;
    _ZnwmRKSt9nothrow_t;
    _ZnwmSt11align_val_t;
    _ZnwmSt11align_val_tRKSt9nothrow_t;

    /* Export std::nothrow just in case (b/130758776).
    */
    _ZSt7nothrow;
  local:
    *;
};

VER_1.0 {
  global:
    extern "C++" {
      platforms::darwinn::api::*;
      platforms::darwinn::internal::*;
      platforms::darwinn::util::*;
      platforms::darwinn::Buffer::*;
    };
  local:
    *;
};

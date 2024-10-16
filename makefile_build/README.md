# Building with only a Makefile

If only building for native systems, it is possible to significantly reduce the complexity of the build by removing Bazel (and Docker). This simple approach builds only what is needed, removes build-time depenency fetching, increases the speed, and uses upstream Debian packages.

To prepare your system, you'll need the following packages (both available on Debian Bookworm / Ubuntu 24.04):
```
sudo apt install libabsl-dev libusb-1.0-0-dev xxd
```

Next, build [FlatBuffers](https://github.com/google/flatbuffers) v23.5.26 required by TensorFlow v2.16.1 from source:

```
git clone --depth 1 --branch v23.5.26 https://github.com/google/flatbuffers.git
cd flatbuffers/
mkdir build && cd build
cmake .. \
    -DFLATBUFFERS_BUILD_SHAREDLIB=ON \
    -DFLATBUFFERS_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

Next, you'll need to clone the [Tensorflow Repo](https://github.com/tensorflow/tensorflow) at the desired checkout (using TF head isn't advised). If you are planning to use libcoral or pycoral libraries, this should match the ones in those repos' WORKSPACE files. For example, if you are using TF2.16.1, we can check that [tag in the TF Repo](https://github.com/tensorflow/tensorflow/tree/v2.16.1) and then checkout that address:
Next, you'll need to clone the [Tensorflow Repo](https://github.com/tensorflow/tensorflow) at the desired checkout (using TF head isn't advised). If you are planning to use libcoral or pycoral libraries, this should match the ones in those repos' WORKSPACE files. For example, if you are using TF2.5, we can check that [tag in the TF Repo](https://github.com/tensorflow/tensorflow/commit/a4dfb8d1a71385bd6d122e4f27f86dcebb96712d) and then checkout that address:
```
git clone --depth 1 --branch v2.16.1 https://github.com/tensorflow/tensorflow
```

To build the library:
```
git clone https://github.com/google-coral/libedgetpu.git
cd libedgetpu
TFROOT=<Directory of Tensorflow> make -f makefile_build/Makefile -j$(nproc)
```

To build packages for Debian/Ubuntu:
```
debuild -us -uc -tc -b -d
```

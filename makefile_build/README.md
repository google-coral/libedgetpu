# Building with only a Makefile

If only building for native systems, it is possible to significantly reduce the complexity of the build by removing Bazel (and Docker). This simple approach builds only what is needed, removes build-time depenency fetching, increases the speed, and uses upstream Debian packages.

To prepare your system, you'll need the following packages (both available on Debian Bookworm / Ubuntu 22.04):
```
sudo apt install libabsl-dev libusb-1.0-0-dev xxd
```

Next, build [FlatBuffers](https://github.com/google/flatbuffers) from source. This is because the version of Flatbuffers provided in the package is different from the version required by TensorFlow v2.16.1.
```
git clone https://github.com/google/flatbuffers.git
cd flatbuffers/
git checkout v23.5.26
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
```
git clone https://github.com/tensorflow/tensorflow.git
cd tensorflow
git checkout git checkout v2.16.1
```

To build the library:  
Note: The `LDFLAGS` flag is required if you specify flatbuffers to be installed to `"/usr/local"`.
```
git clone https://github.com/google-coral/libedgetpu.git
cd libedgetpu/makefile_build
TFROOT=/home/pi/tensorflow/ LDFLAGS="-L/usr/local/lib" make -j$(nproc) libedgetpu
```

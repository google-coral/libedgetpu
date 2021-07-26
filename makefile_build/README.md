# Building with only a Makefile

If only building for native systems, it is possible to significantly reduce the complexity of the build by removing Bazel (and Docker). This simple approach builds only what is needed, removes build-time depenency fetching, increases the speed, and uses upstream Debian packages.

To prepare your system, you'll need the following packages (both available on Debian Bullseye):
```
sudo apt install libabsl-dev libflatbuffers-dev
```

Next, you'll need to clone the [Tensorflow Repo](https://github.com/tensorflow/tensorflow) at the desired checkout (using TF head isn't advised). If you are planning to use libcoral or pycoral libraries, this should match the ones in those repos' WORKSPACE files. For example, if you are using TF2.5, we can check that [tag in the TF Repo](https://github.com/tensorflow/tensorflow/commit/a4dfb8d1a71385bd6d122e4f27f86dcebb96712d) and then checkout that address:
```
git clone https://github.com/tensorflow/tensorflow
git checkout a4dfb8d1a71385bd6d122e4f27f86dcebb96712d -b tf2.5
```

To build the library:
```
TFROOT=<Directory of Tensorflow> make -j$(nproc) libedgetpu
```

# test-cxx-OpenMP-installer

Investigate how to use cmake to link with OpenMP AND install the required libs in an installer.

```shell
mkdir build && cd build
cmake -G Ninja ..
ninja package
chmod +x ./testOMP-0.0.1-Linux.run
```

Then incremental testing:

```shell
/bin/rm -Rf test-install/ && ninja package && ./testOMP-0.0.1-Linux.run  && find . -name "*.so*"
```

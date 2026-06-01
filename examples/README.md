# Examples

This directory includes examples on how to use _sys_sage_ and how to compile source code that links against the _sys-sage_ library.
If _sys-sage_ was installed with a local install prefix, then make sure that the `CMAKE_PREFIX_PATH` is set correctly as described in the root README.

Starting from the project root directory of _sys-sage_, run

```bash
mkdir -p examples/build && cd examples/build
cmake ..
make
```

to build the examples. You can then run them, e.g.

```bash
./basic_usage
```

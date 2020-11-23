# rank-cpp

A header-only library written in C++17 providing generic implementations of key
rank and key enumeration routines.

## Useful notes

`clang-tidy` usage should be akin to the following, assuming the build directory
is `build/`:

```shell
run-clang-tidy -p build/ -header-filter='./include/rankcpp/*' -fix -format
```

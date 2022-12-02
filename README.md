# rank-cpp

A header-only library written in C++17 providing generic implementations of key rank and key enumeration routines. This library is a (slight) improvement over labynkyr and served as basis for the "Comparing Key Rank Estimation Methods" paper (CARDIS 2022), by Rebecca, Luke, and Elisabeth. An easy to use user interface for correlation and template based side channel simulations is provided via the repo ```rank-cpp-experiments```.

## Useful notes

`clang-tidy` usage should be akin to the following, assuming the build directory
is `build/`:

```shell
run-clang-tidy -p build/ -header-filter='./include/rankcpp/*' -fix -format
```

# engine

## Dependency

### MSVC(Windows)

network to access [github](https://www.github.com)

### gcc/clang(linux or mingw)

- glfw
- glm
- assimp

## Run

``` bash
# clone
git clone https://github.com/Hithy/engine.git
cd engine

# compile
cmake -Bbuild -DASSIMP_BUILD_TESTS=off
cmake --build build --config RelWithDebInfo

# run
cmake --build build --config RelWithDebInfo --target run
```
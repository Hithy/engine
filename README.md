# engine

## Feature

- Render Pipeline

	Deferred, PBR, Shadow, SSAO

- Gameplay

	ECS Framework

- Script Binding

	cpython with cxx binding interface

## Screenshot

![gun](resource/images/screenshot/gun.png)
![ball](resource/images/screenshot/pbr.png)
![rotate](resource/images/screenshot/rotate.gif)

## Dependency

### MSVC on Windows

network to access [github](https://www.github.com)

### gcc/clang on Linux or Mingw

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

## Play

ref to [scene.py](script/ecs/scene.py)


# engine

## Feature

- Render Pipeline

	cluster deferred lighting, PBR, TAA, shadow, SSAO, 

- Gameplay

	ECS Framework

- Script Binding

	cpython with cxx binding interface

- Physics

	powered by NVIDIA Physx

## Screenshot

![gun](resource/images/screenshot/gun.png)
![ball](resource/images/screenshot/pbr.png)
![cluster](resource/images/screenshot/cluster1000.png)
![taa](resource/images/screenshot/taa.png)
![taa_detail](resource/images/screenshot/taa_detail.png)
![fog_inside](resource/images/screenshot/fog_inside.png)
![fog_ea](resource/images/screenshot/fog_ea.png)
![rotate](resource/images/screenshot/rotate.gif)
![table](resource/images/screenshot/table.gif)

## Dependency

### MSVC on Windows

network to access github

### gcc/clang on Linux or Mingw

- glfw
- glm
- assimp

## Run (tested on RX 470, Polaris 10, GCN 4)


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


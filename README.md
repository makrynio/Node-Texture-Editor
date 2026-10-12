<img width="1710" height="1072" alt="Screenshot 2025-10-02 at 14 27 25" src="https://github.com/user-attachments/assets/c13da533-3e06-4876-9820-1064c294e782" />
<img width="1710" height="1072" alt="Screenshot 2025-10-02 at 14 33 15" src="https://github.com/user-attachments/assets/43a00189-b42b-4c04-b857-61921f0e175b" />

# Node Texture Editor

A node-based procedural texture editor written in C++17 and OpenGL, with a real-time PBR 3D preview. Build texture graphs by connecting nodes, see the result on a 3D model under image-based lighting, and remesh models with QuadriFlow. Built as a step from using Substance 3D to building the tools behind it.

## Features

- Node graph editor (Dear ImGui and imgui-node-editor) with a context menu to add and connect nodes
- Node types: Texture, Image, Blend, Fractal Noise, Gradient, Tri-Gradient, Mirror, Normal Map, Warp, Better Warp, Tile, Transform 2D, Blur and Output
- Per-node parameter panels
- PBR 3D preview with image-based lighting (irradiance, prefilter and BRDF LUT passes), model loading through Assimp
- Remeshing tool using QuadriFlow, with Manifold for mesh preparation
- Ray-traced view window using Embree, synchronised with the PBR preview (see [RAY_TRACING_README.md](RAY_TRACING_README.md), which predates the current Embree-by-default build)
- macOS native file dialog

## Tech stack

C++17, OpenGL 3.3, GLSL, CMake, Dear ImGui, imgui-node-editor, GLFW, glad, GLM, Assimp, stb_image, Embree, QuadriFlow, Manifold.

## Building

The project is developed on macOS. The notes below follow the `CMakeLists.txt`; the build has not been tested on other platforms.

Requirements: CMake 3.10 or newer, a C++17 compiler, OpenGL, [Assimp](https://github.com/assimp/assimp) (for example `brew install assimp`) and [Embree](https://github.com/RenderKit/embree) 4 (installed system-wide, or built locally with `./install_embree.sh`, which places it in `external/embree_install`).

```bash
git clone --recurse-submodules <this repository>
cd Node-Texture-Editor
```

The submodules provide `external/imgui`, `external/imgui-node-editor` and `external/manifold`. GLFW and glad are not included in this repository: place GLFW in `external/glfw` and a generated glad (OpenGL 3.3 core) in `external/glad`, as expected by `CMakeLists.txt`.

```bash
mkdir build && cd build
cmake ..
make
./ImGuiExample
```

## Project structure

```
nodes/         node implementations and node factory
parameters/    parameter panels for each node
shaders/       PBR, IBL and preview shaders
resources/     sample textures and an HDR environment map
quadriflow_src/ QuadriFlow remeshing source (third party)
quadriflow_3rd/ QuadriFlow dependencies (third party)
include/       GLM, stb_image and LearnOpenGL helper headers
```

## Credits and third-party code

This project builds on the work of others:

- [Dear ImGui](https://github.com/ocornut/imgui) and [imgui-node-editor](https://github.com/thedmd/imgui-node-editor) for the interface and node graph
- [GLFW](https://www.glfw.org/), [glad](https://glad.dav1d.de/), [GLM](https://github.com/g-truc/glm) and [stb_image](https://github.com/nothings/stb) for windowing, OpenGL loading, maths and image loading
- [Assimp](https://github.com/assimp/assimp) for model loading
- [Embree](https://github.com/RenderKit/embree) for the ray-traced view
- [QuadriFlow](https://github.com/hjwdzh/QuadriFlow) for quad remeshing (source included in `quadriflow_src`), together with its bundled dependencies Lemon, MapleCOMSPS_LRB, pcg32 and pss in `quadriflow_3rd`
- [Manifold](https://github.com/hjwdzh/Manifold) for making meshes watertight
- [LearnOpenGL](https://learnopengl.com/) by Joey de Vries: the PBR and image-based lighting shaders and the helper headers in `include/learnopengl` are adapted from the LearnOpenGL tutorial code, and some starter files (`Hello_Window.cpp`, `Triangle.cpp`) follow the tutorials

Each third-party library keeps its own license. Please refer to the respective projects.

## Status

A personal learning and portfolio project. There are no automated tests. The mesh simplification step is a basic triangle removal and is not intended as a production algorithm.

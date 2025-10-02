cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find Assimp
find_package(assimp REQUIRED)

# Find Embree - try system first, then local installation
find_package(embree 4.0 QUIET)
if(NOT embree_FOUND)
    # Try local installation
    set(embree_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/embree_install/lib/cmake/embree-4.4.0")
    find_package(embree 4.0 REQUIRED)
endif()

# Include directories
include_directories(external/glad/include)
include_directories(external/glfw/include)
include_directories(external/imgui)
include_directories(external/imgui/backends)
include_directories(external/imgui-node-editor)
# Include manifold library (simplified)
# include_directories(external/manifold/src)
# include_directories(external/manifold/3rd)
# include_directories(external/manifold/3rd/igl)
include_directories(include)
include_directories(include/glm)
include_directories(src)

# Add GLFW as a subdirectory (exclude examples and tests)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
add_subdirectory(external/glfw)

# QuadriFlow and Lemon libraries removed

# Add GLAD source
add_library(glad external/glad/src/glad.c)

# Add stb_image source
add_library(stb_image src/stb_image.cpp)

# Add texture manager source
add_library(texture_manager src/texture_manager.cpp)

# Add manifold library source (simplified - disabled for now)
# set(MANIFOLD_SOURCES
#     external/manifold/src/BVH.cpp
#     external/manifold/src/Intersection.cpp
#     external/manifold/src/Model_OBJ.cpp
# )

# add_library(manifold_lib ${MANIFOLD_SOURCES})
# target_link_libraries(manifold_lib)

# Add ImGui source files
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui ${IMGUI_SOURCES})
target_link_libraries(imgui glfw OpenGL::GL)

# Add ImGui Node Editor source files
set(IMGUI_NODE_EDITOR_SOURCES
    external/imgui-node-editor/crude_json.cpp
    external/imgui-node-editor/imgui_canvas.cpp
    external/imgui-node-editor/imgui_node_editor.cpp
    external/imgui-node-editor/imgui_node_editor_api.cpp
)

add_library(imgui_node_editor ${IMGUI_NODE_EDITOR_SOURCES})
target_link_libraries(imgui_node_editor imgui)

# Create executable for Triangle
add_executable(Triangle Triangle.cpp)

# Create executable for ImGuiExample
add_executable(ImGuiExample ImGuiExample.cpp OpenGLView.cpp PBRView.cpp PBRViewHelpers.cpp EmbreeView.cpp TriangleRenderer.cpp TextureGenerator.cpp NodeSystem.cpp ContextMenu.cpp ModelLoader.cpp RemeshManager.cpp QuadriflowWrapper.cpp FileDialog.mm
    nodes/BaseNode.cpp nodes/OutputNode.cpp nodes/TextureNode.cpp nodes/ImageNode.cpp nodes/BlendNode.cpp nodes/FractalNoiseNode.cpp nodes/GradientNode.cpp nodes/TriGradientNode.cpp nodes/MirrorNode.cpp nodes/NormalMapNode.cpp nodes/WarpNode.cpp nodes/BetterWarpNode.cpp nodes/TileNode.cpp nodes/Transform2DNode.cpp nodes/BlurV2Node.cpp nodes/NodeFactory.cpp
    parameters/BaseParameter.cpp parameters/OutputNodeParameters.cpp parameters/TextureNodeParameters.cpp parameters/ImageNodeParameters.cpp parameters/FileDialog.mm
    parameters/FractalNoiseParameters.cpp parameters/GradientNodeParameters.cpp parameters/TriGradientNodeParameters.cpp parameters/MirrorNodeParameters.cpp parameters/NormalMapNodeParameters.cpp parameters/WarpNodeParameters.cpp parameters/BetterWarpNodeParameters.cpp parameters/TileNodeParameters.cpp parameters/Transform2DNodeParameters.cpp parameters/BlurV2NodeParameters.cpp parameters/ParametersWindow.cpp)

# FileDialog.mm is already Objective-C++ by extension

# Link libraries
target_link_libraries(Triangle glad glfw OpenGL::GL)
target_link_libraries(ImGuiExample glad glfw imgui imgui_node_editor stb_image texture_manager OpenGL::GL assimp::assimp embree)

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# For macOS
if(APPLE)
    target_link_libraries(Triangle "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    target_link_libraries(ImGuiExample "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-framework AppKit")
endif()

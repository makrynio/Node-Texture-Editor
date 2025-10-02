# Ray-Traced View with Embree

This project now includes a new ImGui window that displays the same scene as your OpenGL PBR view but using Embree for ray tracing.

## Features

- **Synchronized Views**: The ray-traced view automatically syncs with your PBR view, showing the same model, camera position, and lighting
- **Real-time Ray Tracing**: Uses Intel's Embree library for high-performance ray tracing
- **Interactive Controls**: Mouse controls for camera rotation and zoom work the same as the PBR view
- **Model Loading**: Supports the same 3D model formats as the PBR view

## Current Implementation

### Simple Version (Default)
The application currently uses a simple placeholder implementation (`EmbreeViewSimple`) that:
- Renders a basic sphere or loaded model using OpenGL
- Syncs camera and model data with the PBR view
- Provides the same user interface as the full Embree version
- Works without requiring Embree installation

### Full Embree Version
To enable the full ray-traced rendering with Embree:

1. **Install Embree**:
   ```bash
   ./install_embree.sh
   ```

2. **Update CMakeLists.txt**:
   - Uncomment the Embree find_package lines
   - Add `embree` to the target_link_libraries
   - Change `EmbreeViewSimple.cpp` to `EmbreeView.cpp` in the executable

3. **Update ImGuiExample.cpp**:
   - Change `#include "EmbreeViewSimple.h"` to `#include "EmbreeView.h"`
   - Change `EmbreeViewSimple` to `EmbreeView` in the class declarations

## Usage

1. **Enable the View**: In the main menu, go to View → Ray-Traced View
2. **Load Models**: Drag and drop 3D model files onto either the PBR view or ray-traced view
3. **Camera Controls**: 
   - Right-click and drag to rotate the camera
   - Scroll wheel to zoom in/out
   - The camera position syncs between both views
4. **Ray Tracing Controls** (full Embree version):
   - Samples per Pixel: Number of rays per pixel (higher = better quality, slower)
   - Max Ray Depth: Maximum bounces per ray
   - Path Tracing: Enable/disable path tracing

## Technical Details

### EmbreeView Class
- **Scene Building**: Converts loaded 3D models into Embree acceleration structures
- **Ray Tracing**: Primary ray generation and intersection testing
- **Shading**: Simple diffuse shading with environment lighting
- **Synchronization**: Automatically syncs with PBR view data

### Performance Considerations
- Ray tracing is computationally expensive
- Start with low sample counts for real-time interaction
- Higher sample counts provide better quality but slower rendering
- Consider using the simple version for development and the full version for final rendering

## File Structure

```
├── EmbreeView.h              # Full Embree implementation header
├── EmbreeView.cpp            # Full Embree implementation
├── EmbreeViewSimple.h        # Simple placeholder implementation header
├── EmbreeViewSimple.cpp      # Simple placeholder implementation
├── install_embree.sh         # Embree installation script
├── shaders/
│   ├── simple.vs             # Simple vertex shader
│   └── simple.fs             # Simple fragment shader
└── RAY_TRACING_README.md     # This file
```

## Future Enhancements

- **Advanced Shading**: Implement PBR materials in ray tracing
- **Global Illumination**: Add indirect lighting and reflections
- **Denoising**: Implement temporal and spatial denoising for real-time performance
- **Material Editor**: Ray-traced material preview
- **Animation**: Support for animated models and camera paths

## Troubleshooting

### Embree Installation Issues
- Make sure you have CMake and a C++ compiler installed
- Check that the installation script has execute permissions: `chmod +x install_embree.sh`
- Verify the installation path in CMakeLists.txt matches your system

### Compilation Issues
- Ensure all required dependencies are installed (OpenGL, GLFW, ImGui, Assimp)
- Check that the shader files are in the correct location
- Verify that the include paths are correct in CMakeLists.txt

### Runtime Issues
- Check that the application can find the shader files
- Verify that model loading works in the PBR view first
- Check console output for error messages

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

// A minimal model class for this demo
class Model 
{
public:
    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) 
    {
        // For this demo, we don't need actual model loading
        // The demo uses primitive shapes (spheres, cubes, quads) that are generated procedurally
    }

    // draws the model, and thus all its meshes
    void Draw(Shader &shader)
    {
        // Empty implementation for this demo
    }
    
private:
    // Empty implementation for this demo
};
#endif


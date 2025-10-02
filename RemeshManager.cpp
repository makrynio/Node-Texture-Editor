#include "RemeshManager.h"
#include <iostream>
#include <cstdlib>
#include <fstream>

RemeshManager::RemeshManager() {
}

RemeshManager::~RemeshManager() {
}

bool RemeshManager::QuadriflowRemesh(const std::string& inputPath, const std::string& outputPath, const RemeshSettings& settings) {
    std::cout << "=== QUADRIFLOW REMESH ===" << std::endl;
    std::cout << "Input: " << inputPath << std::endl;
    std::cout << "Output: " << outputPath << std::endl;
    std::cout << "Target quads: " << settings.targetQuads << std::endl;
    
    // Check if input file exists and has content
    std::ifstream testInput(inputPath);
    if (!testInput.good()) {
        SetError("Input file does not exist: " + inputPath);
        return false;
    }
    
    // Check input file size
    testInput.seekg(0, std::ios::end);
    std::streampos inputSize = testInput.tellg();
    testInput.close();
    
    if (inputSize == 0) {
        SetError("Input file is empty: " + inputPath);
        return false;
    }
    
    std::cout << "Input file size: " << inputSize << " bytes" << std::endl;
    
    // Remove any existing output file to ensure clean start
    std::remove(outputPath.c_str());
    
    // Run Quadriflow executable (same as Quadriflow_Test)
    std::string quadriflowPath = "/Users/thomasmakryniotis/Desktop/Quadriflow_Test/QuadriFlow/build/quadriflow";
    std::string command = quadriflowPath + " -i " + inputPath + " -o " + outputPath + " -f " + std::to_string(settings.targetQuads) + " 2>&1";
    
    std::cout << "Running: " << command << std::endl;
    int result = system(command.c_str());
    
    if (result != 0) {
        SetError("Quadriflow executable failed with code: " + std::to_string(result) + ". This usually means the input mesh is too complex or contains invalid geometry.");
        return false;
    }
    
    // Check if output file was created and has reasonable content
    std::ifstream testOutput(outputPath);
    if (!testOutput.good()) {
        SetError("Quadriflow output file not created: " + outputPath + ". Quadriflow may have failed silently.");
        return false;
    }
    
    // Check output file size
    testOutput.seekg(0, std::ios::end);
    std::streampos outputSize = testOutput.tellg();
    testOutput.close();
    
    if (outputSize == 0) {
        SetError("Quadriflow created an empty output file. The input mesh may be too complex or contain invalid geometry.");
        return false;
    }
    
    // Validate output file contains valid vertices and no NaN values
    std::ifstream outputFile(outputPath);
    std::string line;
    int validVertexCount = 0;
    int invalidVertexCount = 0;
    int lineCount = 0;
    
    while (std::getline(outputFile, line) && lineCount < 100) { // Check first 100 lines
        lineCount++;
        if (line.substr(0, 2) == "v ") {
            // Check for NaN values in vertex lines
            if (line.find("nan") != std::string::npos || line.find("inf") != std::string::npos) {
                invalidVertexCount++;
            } else {
                validVertexCount++;
            }
        }
    }
    outputFile.close();
    
    std::cout << "Output validation: " << validVertexCount << " valid vertices, " << invalidVertexCount << " invalid vertices in first " << lineCount << " lines" << std::endl;
    
    if (invalidVertexCount > 0) {
        SetError("Quadriflow output contains invalid vertices (NaN/inf values). The input mesh is likely too complex or contains degenerate geometry.");
        return false;
    }
    
    if (validVertexCount == 0) {
        SetError("Quadriflow output contains no valid vertices. The remesh process failed.");
        return false;
    }
    
    std::cout << "Quadriflow remesh completed successfully!" << std::endl;
    std::cout << "Output file size: " << outputSize << " bytes" << std::endl;
    std::cout << "=========================" << std::endl;
    
    return true;
}

void RemeshManager::SetError(const std::string& error) {
    m_LastError = error;
    std::cerr << "RemeshManager Error: " << error << std::endl;
}
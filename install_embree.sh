#!/bin/bash

# Script to install Embree on macOS
# This script downloads and builds Embree from source

echo "Installing Embree for ray tracing..."

# Create external directory if it doesn't exist
mkdir -p external

# Check if Embree is already installed
if [ -d "external/embree" ]; then
    echo "Embree already exists in external/embree"
    exit 0
fi

# Download Embree source
cd external
echo "Downloading Embree source..."
git clone https://github.com/embree/embree.git
cd embree

# Checkout a stable version (4.4.0)
git checkout v4.4.0

# Create build directory
mkdir build
cd build

# Configure with CMake
echo "Configuring Embree build..."
cmake .. -DCMAKE_INSTALL_PREFIX=../../embree_install \
         -DEMBREE_ISPC_SUPPORT=OFF \
         -DEMBREE_TASKING_SYSTEM=INTERNAL \
         -DEMBREE_TUTORIALS=OFF \
         -DEMBREE_EXAMPLES=OFF \
         -DEMBREE_TESTS=OFF

# Build
echo "Building Embree..."
make -j$(nproc)

# Install
echo "Installing Embree..."
make install

echo "Embree installation complete!"
echo "Installation directory: external/embree_install"
echo "You may need to update your CMakeLists.txt to point to this installation."

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Function to show native macOS directory picker
// Returns a malloc'd string that should be freed by the caller
// Returns nullptr if cancelled
const char* ShowDirectoryPicker(const char* lastPath);

// Function to show native macOS file picker for saving
// Returns a malloc'd string that should be freed by the caller
// Returns nullptr if cancelled
const char* ShowSaveFilePicker(const char* title, const char* extension, const char* lastPath);

// Function to show native macOS file picker for opening
// Returns a malloc'd string that should be freed by the caller
// Returns nullptr if cancelled
const char* ShowOpenFilePicker(const char* title, const char* extension, const char* lastPath);

#ifdef __cplusplus
}
#endif

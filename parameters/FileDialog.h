#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Returns a malloc'd string that must be freed with freeFileDialogResult
const char* showOpenFileDialog();
void freeFileDialogResult(const char* result);

#ifdef __cplusplus
}
#endif

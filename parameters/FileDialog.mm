#import <Cocoa/Cocoa.h>
#include <string>

extern "C" {
    const char* showOpenFileDialog() {
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            [panel setTitle:@"Select Image File"];
            [panel setMessage:@"Choose an image file to load"];
            [panel setAllowedFileTypes:@[@"png", @"jpg", @"jpeg", @"bmp", @"tga", @"hdr", @"psd", @"gif", @"pic", @"ppm", @"pgm"]];
            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];
            
            if ([panel runModal] == NSModalResponseOK) {
                NSURL* selectedURL = [[panel URLs] firstObject];
                if (selectedURL) {
                    NSString* path = [selectedURL path];
                    return strdup([path UTF8String]);
                }
            }
            return nullptr;
        }
    }
    
    void freeFileDialogResult(const char* result) {
        if (result) {
            free((void*)result);
        }
    }
}

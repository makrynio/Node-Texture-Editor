#import <AppKit/AppKit.h>
#include <string>

extern "C" {
    const char* ShowDirectoryPicker(const char* lastPath) {
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:NO];
            [panel setCanChooseDirectories:YES];
            [panel setAllowsMultipleSelection:NO];
            [panel setTitle:@"Select Export Directory"];
            [panel setPrompt:@"Select"];
            
            // Set initial directory if we have a saved path
            if (lastPath && strlen(lastPath) > 0) {
                NSString* initialPath = [NSString stringWithUTF8String:lastPath];
                [panel setDirectoryURL:[NSURL fileURLWithPath:initialPath]];
            }
            
            NSInteger result = [panel runModal];
            if (result == NSModalResponseOK) {
                NSURL* selectedURL = [[panel URLs] objectAtIndex:0];
                NSString* path = [selectedURL path];
                return strdup([path UTF8String]);
            }
        }
        return nullptr;
    }
    
    const char* ShowSaveFilePicker(const char* title, const char* extension, const char* lastPath) {
        @autoreleasepool {
            NSSavePanel* panel = [NSSavePanel savePanel];
            [panel setTitle:[NSString stringWithUTF8String:title]];
            [panel setPrompt:@"Save"];
            
            // Set allowed file types
            if (extension && strlen(extension) > 0) {
                NSString* ext = [NSString stringWithUTF8String:extension];
                [panel setAllowedFileTypes:@[ext]];
                [panel setAllowsOtherFileTypes:NO];
            }
            
            // Set initial directory and filename if we have a saved path
            if (lastPath && strlen(lastPath) > 0) {
                NSString* initialPath = [NSString stringWithUTF8String:lastPath];
                NSString* directory = [initialPath stringByDeletingLastPathComponent];
                NSString* filename = [initialPath lastPathComponent];
                [panel setDirectoryURL:[NSURL fileURLWithPath:directory]];
                [panel setNameFieldStringValue:filename];
            }
            
            NSInteger result = [panel runModal];
            if (result == NSModalResponseOK) {
                NSURL* selectedURL = [panel URL];
                NSString* path = [selectedURL path];
                return strdup([path UTF8String]);
            }
        }
        return nullptr;
    }
    
    const char* ShowOpenFilePicker(const char* title, const char* extension, const char* lastPath) {
        @autoreleasepool {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];
            [panel setTitle:[NSString stringWithUTF8String:title]];
            [panel setPrompt:@"Open"];
            
            // Set allowed file types
            if (extension && strlen(extension) > 0) {
                NSString* ext = [NSString stringWithUTF8String:extension];
                [panel setAllowedFileTypes:@[ext]];
            }
            
            // Set initial directory if we have a saved path
            if (lastPath && strlen(lastPath) > 0) {
                NSString* initialPath = [NSString stringWithUTF8String:lastPath];
                NSString* directory = [initialPath stringByDeletingLastPathComponent];
                [panel setDirectoryURL:[NSURL fileURLWithPath:directory]];
            }
            
            NSInteger result = [panel runModal];
            if (result == NSModalResponseOK) {
                NSURL* selectedURL = [[panel URLs] objectAtIndex:0];
                NSString* path = [selectedURL path];
                return strdup([path UTF8String]);
            }
        }
        return nullptr;
    }
}

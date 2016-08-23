/**
 * main.m
 *
 * Description:  Launches the Application.
 *
 * History:
 * bkennedy on 7/05/07 - Created.
 *
 * Copyright MIT 2007 . All rights reserved.
 */

#import <Cocoa/Cocoa.h>

#import <MWorksCore/CoreBuilderForeman.h>
#import <MWorksCore/Exceptions.h>
#import <MWorksCore/StandardServerCoreBuilder.h>


int main(int argc, char *argv[]) {
    try {
        
        mw::StandardServerCoreBuilder coreBuilder;
        mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
        
    } catch (mw::ComponentFactoryConflictException &e) {
        
        @autoreleasepool {
            NSString *error_description = [NSString stringWithCString:e.getMessage().c_str() encoding:NSASCIIStringEncoding];
            NSString *recovery_suggestion = @"You must review your plugins to ensure that multiple plugins aren't trying to register functionality under the same XML signatures";
            NSMutableDictionary *error_info = [[NSMutableDictionary alloc] init];
            [error_info setObject:error_description forKey:NSLocalizedDescriptionKey];
            [error_info setObject:recovery_suggestion forKey:NSLocalizedRecoverySuggestionErrorKey];
            NSError *error = [NSError errorWithDomain:@"PluginLoader"
                                                 code:100
                                             userInfo:error_info];
            [[NSApplication sharedApplication] presentError:error];
        }
        
        return EXIT_FAILURE;
        
    }
    
    return NSApplicationMain(argc, (const char **)argv);
}

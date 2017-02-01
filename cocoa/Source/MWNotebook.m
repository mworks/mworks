//
//  MWNotebook.m
//  New Client
//
//  Created by David Cox on 5/30/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWNotebook.h"
#import "MworksCore/PlatformDependentServices.h"


@implementation MWNotebook

@synthesize content;
@synthesize logFilePath;

- (id)init {
    content = [[NSMutableString alloc] init];
    
    NSString *logFileDir = [[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)
                             objectAtIndex:0] stringByAppendingPathComponent:@"Logs/MWorks"];
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    NSString *logFileName = [[NSDate date] descriptionWithCalendarFormat:@"%Y%m%d_%H%M%S%F.log"
                                                                timeZone:nil
                                                                  locale:nil];
#pragma clang diagnostic pop
    
    logFilePath = [logFileDir stringByAppendingPathComponent:logFileName];
    
    NSLog(@"log file path: %@", logFilePath);

    if (![[NSFileManager defaultManager] createDirectoryAtPath:logFileDir
                                   withIntermediateDirectories:YES
                                                    attributes:nil
                                                         error:NULL] 
        || ![[NSFileManager defaultManager] createFileAtPath:logFilePath contents:nil attributes:nil])
    {
        NSLog(@"log file creation failed");
    } else {
        logFile = [NSFileHandle fileHandleForWritingAtPath:logFilePath];
    }

    return self;
}

- (void) addEntry:(NSString *)entry{
    
    NSMutableString *full_entry = [[NSMutableString alloc] init];
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    NSString *time_string = [[NSDate date] descriptionWithCalendarFormat:@"<%H:%M:%S>: "
                                           timeZone:nil
                                           locale:nil];
#pragma clang diagnostic pop
    
    
    [full_entry appendString:time_string];
    [full_entry appendString:entry];
    [full_entry appendString:@"\n"];
    
    [self willChangeValueForKey:@"content"];
    [content appendString:full_entry];
    [self didChangeValueForKey:@"content"];
    
    [logFile writeData:[full_entry dataUsingEncoding: NSASCIIStringEncoding]];
}



@end

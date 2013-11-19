//
//  NSString+MWorksCocoaAdditions.m
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 11/19/13.
//
//

#import "NSString+MWorksCocoaAdditions.h"


@implementation NSString (MWorksCocoaAdditions)


- (NSString *)mwk_absolutePath {
    NSString *path = [self stringByExpandingTildeInPath];
    if (![path isAbsolutePath]) {
        path = [[[NSFileManager defaultManager] currentDirectoryPath] stringByAppendingPathComponent:path];
    }
    return [path stringByStandardizingPath];
}


@end

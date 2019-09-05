//
//  MWKDatum.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 6/14/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN


typedef NS_ENUM(NSInteger, MWKDatumDataType) {
    MWKDatumDataTypeUndefined,
    MWKDatumDataTypeBoolean,
    MWKDatumDataTypeInteger,
    MWKDatumDataTypeFloat,
    MWKDatumDataTypeByteString,
    MWKDatumDataTypeList,
    MWKDatumDataTypeDictionary
} NS_SWIFT_NAME(Datum.DataType);


NS_SWIFT_NAME(Datum)
@interface MWKDatum : NSObject <NSCopying>

- (instancetype)initWithBool:(BOOL)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithInt:(NSInteger)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithFloat:(double)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithBytes:(NSData *)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithString:(NSString *)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithList:(NSArray<MWKDatum *> *)value NS_REFINED_FOR_SWIFT;
- (instancetype)initWithDict:(NSDictionary<MWKDatum *, MWKDatum *> *)value NS_REFINED_FOR_SWIFT;

@property(nonatomic, readonly) MWKDatumDataType dataType;

@property(nullable, nonatomic, readonly) NSNumber *numberValue NS_REFINED_FOR_SWIFT;
@property(nullable, nonatomic, readonly) NSData *bytesValue;
@property(nullable, nonatomic, readonly) NSString *stringValue;
@property(nullable, nonatomic, readonly) NSArray<MWKDatum *> *listValue;
@property(nullable, nonatomic, readonly) NSDictionary<MWKDatum *, MWKDatum *> *dictValue;

@end


NS_ASSUME_NONNULL_END

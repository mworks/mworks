//
//  MWKDatum.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 6/14/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKDatum_Private.h"


@implementation MWKDatum {
    mw::Datum _datum;
}


+ (instancetype)datumWithBool:(BOOL)value {
    return [[self alloc] initWithBool:value];
}


+ (instancetype)datumWithInt:(NSInteger)value {
    return [(MWKDatum *)[self alloc] initWithInt:value];
}


+ (instancetype)datumWithFloat:(double)value {
    return [(MWKDatum *)[self alloc] initWithFloat:value];
}


+ (instancetype)datumWithBytes:(NSData *)value {
    return [[self alloc] initWithBytes:value];
}


+ (instancetype)datumWithString:(NSString *)value {
    return [[self alloc] initWithString:value];
}


+ (instancetype)datumWithList:(NSArray<MWKDatum *> *)value {
    return [[self alloc] initWithList:value];
}


+ (instancetype)datumWithDict:(NSDictionary<MWKDatum *, MWKDatum *> *)value {
    return [[self alloc] initWithDict:value];
}


- (instancetype)initWithBool:(BOOL)value {
    self = [super init];
    if (self) {
        _datum = mw::Datum(bool(value));
    }
    return self;
}


- (instancetype)initWithInt:(NSInteger)value {
    self = [super init];
    if (self) {
        static_assert(sizeof(NSInteger) == sizeof(long long), "NSInteger and long long have different sizes");
        _datum = mw::Datum(static_cast<long long>(value));
    }
    return self;
}


- (instancetype)initWithFloat:(double)value {
    self = [super init];
    if (self) {
        _datum = mw::Datum(value);
    }
    return self;
}


- (instancetype)initWithBytes:(NSData *)value {
    self = [super init];
    if (self) {
        _datum = mw::Datum(std::string(static_cast<const char *>(value.bytes), value.length));
    }
    return self;
}


- (instancetype)initWithString:(NSString *)value {
    self = [super init];
    if (self) {
        _datum = mw::Datum(value.UTF8String);
    }
    return self;
}


- (instancetype)initWithList:(NSArray<MWKDatum *> *)value {
    self = [super init];
    if (self) {
        mw::Datum::list_value_type list;
        for (MWKDatum *item in value) {
            list.push_back(item.datum);
        }
        _datum = mw::Datum(std::move(list));
    }
    return self;
}


- (instancetype)initWithDict:(NSDictionary<MWKDatum *, MWKDatum *> *)value {
    self = [super init];
    if (self) {
        mw::Datum::dict_value_type dict;
        for (MWKDatum *key in value) {
            dict[key.datum] = value[key].datum;
        }
        _datum = mw::Datum(std::move(dict));
    }
    return self;
}


+ (instancetype)datumWithDatum:(const mw::Datum &)datum {
    return [[self alloc] initWithDatum:datum];
}


- (instancetype)initWithDatum:(const mw::Datum &)datum {
    self = [super init];
    if (self) {
        _datum = datum;
    }
    return self;
}


- (const mw::Datum &)datum {
    return _datum;
}


- (MWKDatumDataType)dataType {
    switch (self.datum.getDataType()) {
        case mw::M_INTEGER:
            return MWKDatumDataTypeInteger;
        case mw::M_FLOAT:
            return MWKDatumDataTypeFloat;
        case mw::M_BOOLEAN:
            return MWKDatumDataTypeBoolean;
        case mw::M_STRING:
            return MWKDatumDataTypeByteString;
        case mw::M_LIST:
            return MWKDatumDataTypeList;
        case mw::M_DICTIONARY:
            return MWKDatumDataTypeDictionary;
        default:
            return MWKDatumDataTypeUndefined;
    }
}


- (NSNumber *)numberValue {
    switch (self.datum.getDataType()) {
        case mw::M_INTEGER:
            return [NSNumber numberWithLongLong:self.datum.getInteger()];
        case mw::M_FLOAT:
            return [NSNumber numberWithDouble:self.datum.getFloat()];
        case mw::M_BOOLEAN:
            return [NSNumber numberWithBool:self.datum.getBool()];
        default:
            return nil;
    }
}


- (NSData *)bytesValue {
    if (self.datum.isString()) {
        auto &string = self.datum.getString();
        return [NSData dataWithBytes:string.data()
                              length:string.size()];
    }
    return nil;
}


- (NSString *)stringValue {
    if (self.datum.isString()) {
        auto &string = self.datum.getString();
        return [[NSString alloc] initWithBytes:string.data()
                                        length:string.size()
                                      encoding:NSUTF8StringEncoding];
    }
    return nil;
}


- (NSArray<MWKDatum *> *)listValue {
    if (self.datum.isList()) {
        NSMutableArray *value = [NSMutableArray array];
        for (auto &item : self.datum.getList()) {
            [value addObject:[[self class] datumWithDatum:item]];
        }
        return value;
    }
    return nil;
}


- (NSDictionary<MWKDatum *, MWKDatum *> *)dictValue {
    if (self.datum.isDictionary()) {
        NSMutableDictionary *value = [NSMutableDictionary dictionary];
        for (auto &item : self.datum.getDict()) {
            [value setObject:[[self class] datumWithDatum:item.second]
                      forKey:[[self class] datumWithDatum:item.first]];
        }
        return value;
    }
    return nil;
}


- (nullable MWKDatum *)objectAtIndexedSubscript:(NSInteger)index {
    MWKDatum *object = nil;
    switch (self.datum.getDataType()) {
        case mw::M_LIST: {
            auto &list = self.datum.getList();
            if (index >= 0 && index < list.size()) {
                object = [[self class] datumWithDatum:list.at(index)];
            }
            break;
        }
            
        case mw::M_DICTIONARY: {
            auto &dict = self.datum.getDict();
            auto iter = dict.find(mw::Datum(static_cast<long long>(index)));
            if (iter != dict.end()) {
                object = [[self class] datumWithDatum:iter->second];
            }
            break;
        }
            
        default:
            break;
    }
    return object;
}


- (MWKDatum *)objectForKeyedSubscript:(NSString *)key {
    MWKDatum *object = nil;
    if (self.datum.isDictionary()) {
        auto &dict = self.datum.getDict();
        auto iter = dict.find(mw::Datum(key.UTF8String));
        if (iter != dict.end()) {
            object = [[self class] datumWithDatum:iter->second];
        }
    }
    return object;
}


- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[MWKDatum class]]) {
        return (self.datum == static_cast<MWKDatum *>(object).datum);
    }
    return NO;
}


- (NSUInteger)hash {
    return self.datum.getHash();
}


- (id)copyWithZone:(NSZone *)zone {
    // MWKDatum is immutable, so just return a new reference to self
    return self;
}


@end

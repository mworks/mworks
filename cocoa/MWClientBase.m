//
//  MWClientBase.m
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/13/19.
//

#import "MWClientBase.h"

#import <MWorksSwift/MWorksSwiftErrors_Private.h>
#import <MWorksSwift/MWKClient_Private.h>

#import "MWClientProtocol.h"
#import "MWCodec.h"
#import "OrderedDictionary.h"


@interface MWClientBase (MWClientProtocolConformance) <MWClientProtocol>
@end


@implementation MWClientBase


- (instancetype)initWithClient:(MWKClient *)client {
    self = [super initWithCore:client];
    if (self) {
        _client = client;
        _notebook = [[MWNotebook alloc] init];
        _variables = [[MWCodec alloc] initWithClientInstance:self];
    }
    return self;
}


- (NSArray *)variableNames {
    // Return a copy of MWCodec's internal variable names array
    return [[self.variables variableNames] copy];
}


@end


@implementation MWClientBase (MWClientProtocolConformance)


- (shared_ptr<mw::Client>)coreClient {
    return self.client.client;
}


- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data {
    // This method must be called from C++ code, so let any exceptions propagate to the caller
    self.client.client->updateValue(code, *data);
}


- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data {
    // This method must be called from C++ code, so let any exceptions propagate to the caller
    [self updateVariableWithCode:[self.client codeForTag:tag] withData:data];
}


- (NSDictionary *)varGroups {
    // Use OrderedDictionary so that iterating over all groups preserves their ordering (which
    // is by order of first appearance in the experiment file)
    OrderedDictionary *allGroups = [[OrderedDictionary alloc] init];
    
    try {
        auto nVars = self.client.client->numberOfVariablesInRegistry();
        for (int i = mw::N_RESERVED_CODEC_CODES; i < nVars + mw::N_RESERVED_CODEC_CODES; i++) {
            auto var = self.client.client->getVariable(i);
            if (var && var->getProperties()) {
                for (auto &group : var->getProperties()->getGroups()) {
                    NSString *groupName = @(group.c_str());
                    NSMutableArray *aGroup = [allGroups valueForKey:groupName];
                    if (!aGroup) {
                        aGroup = [NSMutableArray array];
                        [allGroups setValue:aGroup forKey:groupName];
                    }
                    [aGroup addObject:@(var->getProperties()->getTagName().c_str())];
                }
            }
        }
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    
    return allGroups;
}


@end

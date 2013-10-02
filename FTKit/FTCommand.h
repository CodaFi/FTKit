//
//  FTCommand.h
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class FTOutput;

/**
 * Represents a command to be executed on the Fischer hardware.
 */
@interface FTCommand : NSObject

/**
 * Override point for subclasses.
 *
 * Given an output object, setup any extra state and command internals before execution.
 */
- (void)setup:(FTOutput *)output;

/**
 * The duration the operation should be executed for.
 */
@property (nonatomic) NSTimeInterval duration;

@end

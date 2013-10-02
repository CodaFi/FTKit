//
//  FTOutput.h
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class FTMotorCommand;

@interface FTOutput : NSObject

- (void)buildMotorCommand:(FTMotorCommand *)command;

@end

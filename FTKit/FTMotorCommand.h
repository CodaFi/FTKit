//
//  FTMotorCommand.h
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import <FTKit/FTCommand.h>
#import <FTKit/FTConstants.h>

/**
 * Represents a command to adjust the speed and direction of a motor.  Speed can be any number in the
 * range -7..7 inclusive, with 0 meaning full stop, positive integers indicating "clockwise" motor
 * action and negative numbers representing "counterclockwise" motor actions.  Check hardware
 * connections before debugging speed commands, as crossed wires can reverse the intended direction
 * of a given motor.
 */
@interface FTMotorCommand : FTCommand

/**
 * Convenience initializers for motor commands.
 */
+ (instancetype)motorCommandWithSpeed:(int)speed;
+ (instancetype)motorCommandWithSpeed:(int)speed forPort:(FTMotorPort)port;
+ (instancetype)stopCommandForPort:(FTMotorPort)port; /// Motor speed 0.

- (id)initWithSpeed:(int)speed;
- (id)initWithSpeed:(int)speed forPort:(FTMotorPort)port;

@property (nonatomic) FTMotorPort port;
@property (nonatomic) int speed; 

@end

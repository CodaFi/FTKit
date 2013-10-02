//
//  FTMotorCommand.m
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import "FTMotorCommand.h"
#import "FTOutput.h"

using namespace ftapi;

@implementation FTMotorCommand {
	FtMotorSpeed _speedCommand;
}

+ (instancetype)motorCommandWithSpeed:(int)speed {
	return [[self alloc] initWithSpeed:0];
}

+ (instancetype)motorCommandWithSpeed:(int)speed forPort:(FTMotorPort)port {
	return [[self alloc] initWithSpeed:speed forPort:port];
}

+ (instancetype)stopCommandForPort:(FTMotorPort)port {
	return [[self alloc] initWithSpeed:0 forPort:port];
}

- (id)init {
	return [self initWithSpeed:0];
}

- (id)initWithSpeed:(int)speed {
	self = [super init];
	
	_speed = speed;
	_port = FTMotorPort1;
	
	return self;
}

- (id)initWithSpeed:(int)speed forPort:(FTMotorPort)port {
	self = [super init];
	
	_speed = speed;
	_port = (FTMotorPort)port;
	
	return self;
}

- (void)setup:(FTOutput *)output {
	[output buildMotorCommand:self];
}

- (ftapi::FtMotorSpeed)motorSpeed {
	return _speedCommand;
}

@end

//
//  FTOutput.m
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import "FTOutput.h"
#import "FTMotorCommand+Private.h"

using namespace ftapi;

@implementation FTOutput {
	IfOutputs _outputs;
}

- (id)init {
	self = [super init];
	
	_outputs.data[FT_MASTER]= 0xFF;
	
	return self;
}

- (void)buildMotorCommand:(FTMotorCommand *)command {
	_outputs.setMotorSpeed(FT_MASTER, (ft_motor)command.port, command.motorSpeed);
}

- (ftapi::IfOutputs)outputs {
	return _outputs;
}

@end

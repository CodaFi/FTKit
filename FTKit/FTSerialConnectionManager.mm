//
//  FTSerialConnectionManager.m
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import "FTSerialConnectionManager.h"
#import "FTConstants.h"
#import "FTOutput+Private.h"
#import "FTCommand.h"
#include <iostream>

using namespace std;
using namespace ftapi;

/// specifies connection type
static const char *FTConnectionDescriptionTable[] {
	/// FTConnectionTypeUnknown,
	"Unknown",
	/// FTConnectionTypeSerial,
	"Serial",
	/// FTConnectionTypeUSB,
	"USB",
	/// FTConnectionTypeIOKit
	"I/O Kit",
	/// FTConnectionTypeLibUSB
	"libUSB",
};

const char *FTDefaultUSBDeviceName = "/dev/usb/robointerface0";


@implementation FTSerialConnectionManager {
	FtInterface *fischerInterface;
	bool _loggingEnabled;
}

+ (instancetype)defaultManager {
	static FTSerialConnectionManager *manager;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		manager = [[FTSerialConnectionManager alloc] init];
	});
	return manager;
}

- (id)init {
	self = [super init];
		
	return self;
}

- (void)dealloc {
	if (fischerInterface != NULL) {
		delete fischerInterface;
	}
}

- (BOOL)connect {
	return [self connectToDeviceNamed:""];
}

- (BOOL)connectToDeviceNamed:(const char *)name {
	return [self openFirstAvailableConnectionToDevice:name ?: ""];
}

- (BOOL)openFirstAvailableConnectionToDevice:(const char *)name {
	string device(name);
	ft_if_conn connection = FT_IC_UNKNOWN;
	FtInterface *fti;
	do {
		try {
			fti = ftGetIfFromFactory(device, connection);
			NSLog(@"Valid interface found for device.");
			fischerInterface = fti;
			return YES;
		} catch (exception &e) {
			delete fti;
			continue;
		}
	} while (connection++ < FT_IC_LIB_USB);
	return NO;
}

- (NSString *)executeCommands:(FTCommand *)commands, ... {
	if (fischerInterface == NULL) return nil;
	
	va_list args;
	va_start(args, commands);
	
	NSMutableArray *commandList = [[NSMutableArray alloc] init];
	for (FTCommand *com = commands; com != nil; com = va_arg(args, FTCommand *)) {
		[commandList addObject:com];
	}
	va_end(args);
	return [self executeCommandsInArray:commandList];
}

- (NSString *)executeCommandsInArray:(id<NSFastEnumeration>)commands {
	if (fischerInterface == NULL) return nil;

	FTOutput *output = [[FTOutput alloc] init];
	NSMutableString *outputString = @"".mutableCopy;
	
	try {
		for (FTCommand *command in commands) {
			[command setup:output];
			IfInputs inp;
			NSInteger i = 0;
			NSInteger lim = ceil(command.duration/.300);
			while (i++ <= lim) {
				fischerInterface->writeAndReadAllData(output.outputs, inp);
			}
			[outputString appendFormat:@"Results of %@:\n\
			 Supply: %g Voltz\n\
			 AX: %d Ohms \n\
			 AY: %d Ohms \n\
			 \n\n", command, (inp.supply * 0.0269), inp.ax[FT_MASTER], inp.ay];
		}
	} catch (exception &e) {
		cout << e.what() << endl;
		[[NSException exceptionWithName:@"FTCommandFailureException"
								reason:@"A command sent to the Fischer interface resulted in an exception."
							  userInfo:@{
										 NSLocalizedDescriptionKey : [NSString stringWithCString:e.what() encoding:NSUTF8StringEncoding]
										}] raise];
	}
	return outputString;
}

- (NSString *)description {
	if (fischerInterface == NULL) {
		return [NSString stringWithFormat:@"<%@: %@>", self.class, self];
	}
	
	IfInfo info;
	fischerInterface->getIfInfo(info);
	
	return [NSString stringWithFormat:@"<%@: %@> \n\
									   Connection established to device named %s (Serial: %d)\n\
									   Connected via %s \n\
									   ", self.class, self, info.device.c_str(), info.serialNo, FTConnectionDescriptionTable[info.ifConn]];
}

@end

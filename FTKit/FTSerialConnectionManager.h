//
//  FTSerialConnectionManager.h
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class FTCommand, FTCommandAggregate;

extern const char *FTDefaultUSBDeviceName;

/**
 * Wraps the C++ interface to a Fishertecniks robotic interface and provides a dispatch point for 
 * commands to be sent to the robot.  All connections are serial and must be run on the main thread.
 * Failure to guarantee the thread of execution can result in hardware errors or device malfunctions.
 */
@interface FTSerialConnectionManager : NSObject

/**
 * Returns the default connection manager for all Fisher interfaces.  The singleton will attempt to
 * autoconnect to the first interface it finds with the default empty device name.
 */
+ (instancetype)defaultManager;

/**
 * Attempts to establish a connection to the first robo interface it finds.  If matches are found, 
 * this method returns YES, else NO.
 */
- (BOOL)connect;

/**
 * Attempts to establish a connection to the first robo interface it finds matching the given name
 * or serial number.  If matches are found, this method returns YES, else NO.
 * 
 * Robo interfaces connected via USB on Linux and Windows should use this method in conjunction with
 * FTDefaultUSBDeviceName of they do not wish to provide a device name to search for.
 */
- (BOOL)connectToDeviceNamed:(const char *)name;

/**
 * Executes a variadic number of command objects on the Fisher interface for their given durations
 * and dumps the results of each command along with information about the current interface into
 * the resulting string.
 *
 * Returns nil if a fischer interface is either disconnected or unavailable.
 */
- (NSString *)executeCommands:(FTCommand *)commands,  ... NS_REQUIRES_NIL_TERMINATION;

/**
 * Executes an array of command objects on the Fisher interface for their given durations
 * and dumps the results of each command along with information about the current interface into
 * the resulting string.
 *
 * Returns nil if a fischer interface is either disconnected or unavailable.
 */
- (NSString *)executeCommandsInArray:(id<NSFastEnumeration>)commands;

@end

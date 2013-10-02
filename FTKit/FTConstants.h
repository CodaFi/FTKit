//
//  FTConstants.h
//  FTKit
//
//  Created by Robert Widmann on 10/2/13.
//  Copyright (c) 2013 CodaFi. All rights reserved.
//

#ifndef FTKit_FTConstants_h
#define FTKit_FTConstants_h

/// specifies connection type
typedef enum {
	/// unknown connection type (not properly set)
	FTConnectionTypeUnknown,
	/** RoboInterface (serial: 38400,8,n,1);
	 Intelligent Interface (serial: 9600,8,n,1) */
	FTConnectionTypeSerial,
	/// RoboInterface (usb, fischerusb kernel driver req.)
	FTConnectionTypeUSB,
	/// RoboInterface (usb, MacOSX IOKit userspace access)
	FTConnectionTypeIOKit,
	/// RoboInterface via libusb
	FTConnectionTypeLibUSB
} FTConnectionType;

/// specifies interface model
typedef enum {
	/// unknown device (not properly set?)
	FTInterfaceTypeUnknown,
	/** (old) Intelligent Interface (or Robo Interface in
	 "compatibility mode" */
	FTInterfaceTypeIntelligentInterface,
	/// (new) Robo Interface
	FTInterfaceTypeRoboInterface,
	/// standalone Robo I/O Extension (usb)
	FTInterfaceTypeRoboIO,
	/// Robo RF Data (usb)
	FTInterfaceTypeRFData,
	/// Sound+Lights
	FTInterfaceTypeSoundAndLights
} FTInterfaceType;

typedef enum {
	/// Motor1
	FTMotorPort1 = 0,
	/// Motor2
	FTMotorPort2 = 1,
	/// Motor3
	FTMotorPort3 = 2,
	/// Motor4
	FTMotorPort4 = 3
} FTMotorPort;

#endif

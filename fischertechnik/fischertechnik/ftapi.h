/***************************************************************************
 *  ftApi - fischertechnik c++ progamming api                              *
 *  Copyright (C) 2004,2005 by                                             *
 *  Holger Friedrich <holgerf@vsi.cs.uni-frankfurt.de>,                    *
 *  Visual Sensorics and Information processing lab,                       * 
 *  University of Frankfurt                                                *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; if not, write to the Free Software    *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA02110-1301 USA*
 ***************************************************************************/
#ifndef FTAPI_H
#define FTAPI_H

/** @file
public header file */

#include <exception>
#include <string>

#ifdef WIN32
#pragma warning( disable : 4290 )
#endif

/// namespace containing fischer stuff
namespace ftapi {
  /** base exception class. 
  
  @note All exceptions are (directly or indirectly) inherited
  from std::exception. 
  
  They can be caught the following way:
  @code 
  try {
    ...
    // this may throw exceptions...
    ...
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  @endcode
  */
  class XFt: public std::exception {
    public:
    XFt();
    XFt(std::string message);
    virtual ~XFt() throw();
    virtual const char* what() const throw();
    protected: 
    std::string msg;
  };
  /// communication exception
  class XFtComm: public XFt {
    public:
    XFtComm();
    XFtComm(std::string message);
    virtual ~XFtComm() throw();
    virtual const char* what() const throw();
  };
  /// communication timeout
  class XFtCommTimeout: public XFtComm {
  public:
    XFtCommTimeout();
    XFtCommTimeout(std::string message);
    virtual ~XFtCommTimeout() throw();
    virtual const char* what() const throw();
  };

  /// specifies connection type
  enum ft_if_conn {
    /// unknown connection type (not properly set)
    FT_IC_UNKNOWN,  
    /** RoboInterface (serial: 38400,8,n,1); 
        Intelligent Interface (serial: 9600,8,n,1) */
    FT_IC_SERIAL,  
    /// RoboInterface (usb, fischerusb kernel driver req.)
    FT_IC_USB,
    /// RoboInterface (usb, MacOSX IOKit userspace access)
    FT_IC_USB_IOKIT,
    /// RoboInterface via libusb
    FT_IC_LIB_USB
  };

  /// specifies interface model
  enum ft_if_type {
    /// unknown device (not properly set?)
    FT_IF_UNKNOWN,  
    /** (old) Intelligent Interface (or Robo Interface in
    "compatibility mode" */ 
    FT_IF_INTELLI,
    /// (new) Robo Interface
    FT_IF_ROBO,
    /// standalone Robo I/O Extension (usb)
    FT_IF_IOE,
    /// Robo RF Data (usb)
    FT_IF_RF_DATA,
    /// Sound+Lights
    FT_IF_SAL
  };
 
  /** enum to specify interface or one of it's extensions. */
  enum ft_module_type {
    /// interface itself
    FT_MASTER = 0,
    /// I/O extension 1 (Robo Interface in Intelligent Interface) 
    FT_SLAVE1 = 1,
    /// I/O extension 2 (Robo Interface only) 
    FT_SLAVE2 = 2,
    /// I/O extension 3 (Robo Interface only) 
    FT_SLAVE3 = 3
  };
 
  /** enum to specify the motornumber. */
  enum ft_motor {
    /// Motor1
    FT_M1 = 0,
    /// Motor2
    FT_M2 = 1,
    /// Motor3
    FT_M3 = 2,
    /// Motor4
    FT_M4 = 3
  };

  /** enum to specify digital inputs I1..I8 
  @see ftapi::IfInputs::pin()
  */
  enum ft_pin {
    FT_I1 = 1<<0,
    FT_I2 = 1<<1,
    FT_I3 = 1<<2,
    FT_I4 = 1<<3,
    FT_I5 = 1<<4,
    FT_I6 = 1<<5,
    FT_I7 = 1<<6,
    FT_I8 = 1<<7
  };
 
  /// specifies program locations
  enum ft_program_location {
    FT_RAM = 2,
    FT_FLASH1 = 0,
    FT_FLASH2 = 1
  };
 
  /// RoboInterface inputs 
  class IfInputs {
    public:
    /** ctor. @note initializes all values to 0. */
    IfInputs();

    /// @name digital inputs
    //@{
    /** digital input I1-8 of master and up to 3 I/O extensions.
    Use ft_module_type enum to specify master or one of the slaves. 
    @see ftapi::IfInputs::pin() (simple access to one digital input)
    */ 
    unsigned char data[4]; 
    /** check if pin is set.
    This is an easy way to check one digital input:
    @code 
      IfInputs inp;
      ...
      if (inp.pin(FT_MOTOR, I2)) { ... }
    @endcode

    @see ftapi::IfInputs::data to get access to all 8 digital inputs at 
    once (one byte)
    */
    bool pin(ft_module_type module, ft_pin pin); 
    //@}
    
    /// @name analog inputs
    //@{
    /** analog input AX (electric resistance, 0-5k ohm) of master and 
    up to 3 I/O extensions.
    Use ft_module_type enum to specify master or one of the slaves. */
    int ax[4]; 
    /** analog input AY (electric resistance, 0-5k ohm) of master. */
    int ay;
    /** undocumented */
    int az;
    /** analog input A1 of master (voltage, 0-10 volts). 
    Multiply by factor 1/100 to get volts. */
    int a1;
    /** analog input A2 of master (voltage, 0-10 volts).
    Multiply by factor 1/100 to get volts. */
    int a2;
    /** supply voltage. Multiply by factor 0,0269 to get volts. */
    int supply;
    //@}
    
    /// @name distance sensors
    //@{
    /** digital distance sensor AS1 present */
    bool as1Digital;
    /** digital distance sensor AS2 present */
    bool as2Digital;
    /** digital distance sensor D1 measurement */
    int dist1;
    /** digital distance sensor D2 measurement */
    int dist2;
    //@}

    /** time stamp (milliseconds) */
    unsigned char timeInMs;
    /** @name infrared */
//@{
    /** keycode (infrared), key 1..11 pressed */
    int irKey;
    /** undocumented, code1 (false) or code2 (true). */
    bool code2;    
//@}
  };
  
  class FtMotorSpeed;
    
  /** RoboInterface outputs.
  Each fischertechnik device has 8 motor outputs. They can be used to control
  4 motors (2 outputs for each motor): 0,1: M1, 2,3: M2, 4,5: M3, 6,7: M4.
  Motor control is done the following way:
  - activate output ports (field data) for the motor by seting the bits 0,1: M1, 
    2,3: M2, 4,5: M3, 6,7: M4.
  - set speed value: set one output to 0, the other 0..7 to regulate speed from
    slow to fast (both 0: stop). Swap ports to change motor direction.
 
  Example:
  @code
  IfOutputs out;
  // now try to start M1 connected to Robo Interface:
  out.data[FT_MASTER] |= (1<<0 + 1<<1);  // M1: set bit 0 and 1 to 1
  out.speed[FT_MASTER][0] = 7;  // 7,0: 
  out.speed[FT_MASTER][1] = 0;  // fast forward
  ...
  out.speed[FT_MASTER][0] = 0;  // 0,3: 
  out.speed[FT_MASTER][1] = 3;  // slow backwards
  ...
  
  // now try to start M4 connected to Robo Interface:
  out.data[FT_MASTER] |= (1<<6 + 1<<7);  // M4: bit 0 and 1
  out.speed[FT_MASTER][6] = 7;  // 7,0: 
  out.speed[FT_MASTER][7] = 0;  // fast forward
  @endcode
  
  To activate all motor outputs you can simply write
  @code
  out.data[FT_MASTER] = 0xff;
  @endcode
  ...and then set the speeds.
  */ 
  class IfOutputs {
    public:
    /** ctor. @note initializes all values to 0. */
    IfOutputs();
    
    /** output ports of master or one of the extensions. 
    8 motor outputs (+/--voltage-plugs) can be used to control 4 motors: 0,1: M1, 2,3: M2, 4,5: M3, 6,7: M4.
    Use ftapi::ft_module_type enum to specify master or one of the slaves. */
    unsigned char data[4];
    
    /** motor speed (allowed range: slow..fast 0..7) of master or one of the extensions.
    8 motor outputs (+/--voltage-plugs) can be used to control 4 motors: 0,1: M1, 2,3: M2, 4,5: M3, 6,7: M4.
    Use ftapi::ft_module_type enum to specify master or one of the slaves. 
    @note you have to set corresponding bits in ftapi::IfOutputs.data to activate motor output.*/ 
    unsigned char speed[4][8];
    
    /** Sets the motor speed and activates corresponding bits in ftapi::IfOutputs.data to switch on motor.
    \param module Specify output ports of master or one of the extensions
    \param no Motor number: use ftapi::ft_motor enum to specify M1..M4
    \param mSpeed Motor speed: use an object of type ftapi::FtMotorSpeed
    @note If you do not need a ftapi::FtMotorSpeed object you can use an std::int instead here. Then a temporary object will be implicitely created and thrown away after the speed is set. But in this case you can not manipulate and compare the motor speed of course. */
    void setMotorSpeed(const ft_module_type module, const ft_motor no, const FtMotorSpeed &mSpeed);


    FtMotorSpeed getMotorSpeed(const ft_module_type module, const ft_motor no) const;
  };

  
  /** general information about device (connection, model, etc) 
    @note Change from v0.1 to v0.2: removed slavePresent[3], 
    radioCommModulePresent, internetModulePresent because detection 
    of extensions currently does not work.
  */
  class IfInfo {
    public:
    /** ctor. @note initializes all values to 0. */
    IfInfo();

    ft_if_type ifType;           ///< interface model  
    ft_if_conn ifConn;           ///< connection type  
    std::string device;          ///< filename and path  
    /** firmware, bootloader version. Format: F.W.B.L. (0..3).
    @note Only Robo Interface with serial connection provides this
    information correctly. Robo Interface with USB connection
    currently will retrieve rev. number as shown by 
    "cat /proc/bus/usb/devices"; this number seems to differ.
    Older Intelligent Interface is not
    supported (0.0.0.0). */  
    unsigned char firmwareVer[4];
    /** serial number. 
    @note Robo Interface might return serial no "1", which is a
    kind of "standard serial no" for all interfaces. You can use
    the RoboPRO software to change this behavior and enable the 
    serial which is printed on the interface. This will also affect
    "cat /proc/bus/usb/devices". */   
    int serialNo;
    /* we are unable to detect extensions: 
    the following code does not work properly
    slaves (I/O extensions) present.
    @note Will only work for Robo Interface.
    Don't know how to detect extensions of the Intelligent Interface.
    bool slavePresent[3];        ///< slaves present?
    bool radioCommModulePresent; ///< ROBO RF Data present?
    bool internetModulePresent;  ///< undocumented
    */
    unsigned char rfdFirmwareVer[4];
    int rfdSerialNo;
    /// RF Data: radio frequency setting (2..80)
    int rfdFrequency;
    /// name of program stored in FLASH1 (Robo Interface); sound file 1 (Sound+Lights)
    std:: string filenameFlash1;
    /// name of program stored in FLASH2 (Robo Interface); sound file 2 (Sound+Lights)
    std:: string filenameFlash2;
    /// sound file 3 (Sound+Lights)
    std:: string filenameFlash3;
    /// name of program stored in RAM
    std:: string filenameRam;
  };


  /** Abstract base class for accessing fischertechnik interfaces.
  This base class has been provided to hide the different ways how
  to connect a fischertechnik device to your PC (USB, serial cable,
  ...).

  A factory function ftapi::ftGetIfFromFactory provides instances
  of the appropriate classes for the given interface type and
  connection.
  @see ftapi::ftGetIfFromFactory
  */
  class FtInterface {
    public: 
    /// dtor
    virtual ~FtInterface() {};
    
    /// gather general information on the connected interface.
    virtual void getIfInfo(IfInfo &info) = 0;
   
    /** THIS FUNCTION MUST BE CALLED VERY OFTEN - it sets all outputs and reads all inputs. 
    How to set outputs is explained at ftapi::IfOutputs;
    see ftapi::IfInputs for the explaination of the interface inputs.
    @note This function must be called at least every 300ms, else motors will stop working
    until next call (this is kind of emergency breaking, the robots just stops working when
    communication breaks down).
    @note This function may block. 
    @throws XFtComm on serious errors
    @throws XFtCommTimeout if RF Data loses RF connection
    */ 
    virtual void writeAndReadAllData(const IfOutputs &outputs, IfInputs
&inputs) throw (XFtComm) = 0;
    
    /** start a program which has been downloaded to the RoboInterface
    @param location specifies which program to start
    @note RoboInterface only 
    @see stopExecution() */
    virtual void startExecution(ft_program_location location) = 0;
    /** stop a program which has been downloaded to the RoboInterface
    @note RoboInterface only 
    @see startExecution() */
    virtual void stopExecution() = 0;
    /** query if a program is executed 
    @param plocation if stopExecution returns true, plocations returns which program is executed
    @returns true if a program is executed, otherwise interface is in online mode
    */
    virtual bool isExecutionActive(ft_program_location *plocation = 0) = 0;
    

    virtual void storeWavFiles(std::string fn1, std::string fn2, std::string fn3) = 0;
    virtual void deleteFlash() = 0;
  };
  
  /** A factory function to provide instances
  of the appropriate classes for the given interface type and
  connection.
  The different classes are completely hidden.

  Instances can be created by the factory this way:
  @code
  // usb driver fischerusb (linux 2.6):
  FtInterface *ifusb = ftGetIfFromFactory("/dev/usb/robointerface0", FT_IC_USB);

  // generic usb driver using libusb:
  // 5312 is the serial number returned from the device
  // use "" to match any serial
  FtInterface *iflibusb = ftGetIfFromFactory("5312", FT_IC_LIB_USB);

  // native Apple IO-Kit
  // (device naming looks like libusb device naming above)
  FtInterface *ifusbiokit = ftGetIfFromFactory("5312", FT_IC_USB_IOKIT);

  // serial connection 
  FtInterface *ifserial = ftGetIfFromFactory("/dev/ttyS0", FT_IC_SERIAL);
  ...
  delete(ifserial);
  delete(ifusbiokit);
  delete(iflibusb);
  delete(ifusb);
  @endcode

  Do not forget to destruct the instances after work (delete). An elegant way to
  do this automatically is using the std::auto_ptr mechanism:
  @code
  #include <memory>
  ...
  try {
    // open a connection to fischertechnik interface
    // (auto_ptr provides automatic garbage collection, ptr will be released
    // when reaching end of block)
    std::auto_ptr<ftapi::FtInterface> fti (
     ftapi::ftGetIfFromFactory("5312", FT_IC_LIB_USB));

    // show some information on the fischertechnik interface
    ftapi::IfInfo ii;
    fti->getIfInfo(ii);
    std::cout << ii << std::endl;
 
    // when leaving this block (exception or regular exit), fti will be released
  } catch ... 
  @endcode
  @throws XFt if connection tpye (backend) is not supported or device cannot 
  be opened. See @ref backends for a complete list of devices and backends
  supported by ftapi. See also INSTALL how to enable backends.

  @param device
  depends on backend:
  - libusb: Serial number returned by device. Special strings are "*" 
    or "", which match any serial returned by device. If you have several
    devices returning "standard serial" set by fischer, which is always "1",
    the first interface which can be opened (i.e. it is not already opened
    by another call) is returned. 
  - fischerusb: Name of the device node, e.g. /dev/usb/robointerface0
  - Apple IO-Kit: Serial number returned by device. NOTE: currently not
    implemented -> returns first device which can be openend.
  - serial connection: Name of the device node, e.g. /dev/ttyS0 
  @param connection connection type, see ftapi::ft_if_conn
  */
  FtInterface *ftGetIfFromFactory(std::string device, ft_if_conn connection) throw (XFt);
  
  /** Datatype which stores and manipulates the speed of a motor.
  The range is from _minSpeed (here: -7 = backwards) to _maxSpeed (here: 7 = forewards), 
  values larger or smaller respectively will be clipped. You have to build a ftapi::FtMotorSpeed object 
  for each motor and use it as a parameter in the method ftapi::IfOutputs.setMotorSpeed.*/
  class FtMotorSpeed {
    public:
    /** ctor. @note initializes speed to 0 if no parameter given. */
    FtMotorSpeed(int speed=0);

    void stop();           ///< stop motor, set speed to 0
    
    /** Set speed in range _minSpeed to _maxSpeed. speed will be clipped to _maxSpeed and _minSpeed 
    respectively if the given value is to large or to small. */
    void set(const int speed);
    int get() const;       ///< returns the current speed
    void setMax();         ///< set speed to _maxSpeed
    void setMin();         ///< set speed to _minSpeed
    
    /** Increments motorspeed by 1. It will be clipped at _maxSpeed; postfix, e.g. mSpeed1++ */
    const FtMotorSpeed operator++(const int);
    
    /** Decrements motorspeed by 1. It will be clipped at _minSpeed; postfix, e.g. mSpeed1-- */
    const FtMotorSpeed operator--(const int);
    
    /** Increments motorspeed by 1. It will be clipped at _maxSpeed; prefix, e.g. ++mSpeed1 */
    FtMotorSpeed& operator++();
    
    /** Decrements motorspeed by 1. It will be clipped at _minSpeed; prefix, e.g. --mSpeed1 */
    FtMotorSpeed& operator--();
    
    /** Increments motorspeed by the value of speed. 
    It will be clipped at _maxSpeed; e.g. mSpeed1+=3 */
    void operator+=(const int speed);
    
    /** Decrements motorspeed by the value of speed. 
    It will be clipped at _minSpeed; e.g. mSpeed1-=3 */
    void operator-=(const int speed);
    
    /** Returns motorspeed incremented by the value of speed. The object itself will not be changed! 
    It will be clipped at _maxSpeed; e.g. mSpeed1 = mSpeed2 + 3 */
    FtMotorSpeed operator+(const int speed);
    
    /** Returns motorspeed decremented by the value of speed. The object itself will not be changed! 
    It will be clipped at _minSpeed; e.g. mSpeed1 = mSpeed2 - 3 */
    FtMotorSpeed operator-(const int speed);
    
    /** Set speed in range _minSpeed to _maxSpeed. Speed will be clipped to 
    _maxSpeed and _minSpeed respectively if the given value is to large or to small. */
    FtMotorSpeed& operator=(const int speed);
    
    /** Set speed with speed of another motor. */
    FtMotorSpeed& operator=(const FtMotorSpeed &speed);
    
    /** Comparison of two speedobjects: True if the objects are of the same speed. */
    bool operator==(const FtMotorSpeed speed);
    
    /** Comparison of two speedobjects: True if the objects are of different speed. */
    bool operator!=(const FtMotorSpeed speed);
    
    /** Comparison of two speedobjects. */
    bool operator<(const FtMotorSpeed speed);
    
    /** Comparison of two speedobjects. */
    bool operator>(const FtMotorSpeed speed);
    
    /** Comparison of two speedobjects. */
    bool operator<=(const FtMotorSpeed speed);
    
    /** Comparison of two speedobjects. */
    bool operator>=(const FtMotorSpeed speed);
   
     
    private:
    int _speed;         ///< current speed
    static const int _maxSpeed = 7;
    static const int _minSpeed = -7;
    
  };


}

/// easy way to print information about interface to a stream
std::ostream& operator<<(std::ostream& s, const ftapi::IfInfo &x);

#endif
